// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/template.h"

#include "tools/gn/err.h"
#include "tools/gn/functions.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/scope.h"
#include "tools/gn/scope_per_file_provider.h"
#include "tools/gn/value.h"

Template::Template(const Scope* scope, const FunctionCallNode* def)
    : closure_(scope->MakeClosure()),
      definition_(def) {
}

Template::Template(scoped_ptr<Scope> scope, const FunctionCallNode* def)
    : closure_(scope.Pass()),
      definition_(def) {
}

Template::~Template() {
}

scoped_ptr<Template> Template::Clone() const {
  // We can make a new closure from our closure to copy it.
  return scoped_ptr<Template>(
      new Template(closure_->MakeClosure(), definition_));
}

Value Template::Invoke(Scope* scope,
                       const FunctionCallNode* invocation,
                       const std::vector<Value>& args,
                       BlockNode* block,
                       Err* err) const {
  // Don't allow templates to be executed from imported files. Imports are for
  // simple values only.
  if (!EnsureNotProcessingImport(invocation, scope, err))
    return Value();

  // First run the invocation's block. Need to allocate the scope on the heap
  // so we can pass ownership to the template.
  scoped_ptr<Scope> invocation_scope(new Scope(scope));
  if (!FillTargetBlockScope(scope, invocation,
                            invocation->function().value().as_string(),
                            block, args, invocation_scope.get(), err))
    return Value();
  block->ExecuteBlockInScope(invocation_scope.get(), err);
  if (err->has_error())
    return Value();

  // Set up the scope to run the template and set the current directory for the
  // template (which ScopePerFileProvider uses to base the target-related
  // variables target_gen_dir and target_out_dir on) to be that of the invoker.
  // This way, files don't have to be rebased and target_*_dir works the way
  // people expect (otherwise its to easy to be putting generated files in the
  // gen dir corresponding to an imported file).
  Scope template_scope(closure_.get());
  template_scope.set_source_dir(scope->GetSourceDir());

  ScopePerFileProvider per_file_provider(&template_scope, true);

  // Targets defined in the template go in the collector for the invoking file.
  template_scope.set_item_collector(scope->GetItemCollector());

  // We jump through some hoops to avoid copying the invocation scope when
  // setting it in the template scope (since the invocation scope may have
  // large lists of source files in it and could be expensive to copy).
  //
  // Scope.SetValue will copy the value which will in turn copy the scope, but
  // if we instead create a value and then set the scope on it, the copy can
  // be avoided.
  const char kInvoker[] = "invoker";
  template_scope.SetValue(kInvoker, Value(NULL, scoped_ptr<Scope>()),
                          invocation);
  Value* invoker_value = template_scope.GetMutableValue(kInvoker, false);
  invoker_value->SetScopeValue(invocation_scope.Pass());
  template_scope.set_source_dir(scope->GetSourceDir());

  const base::StringPiece target_name("target_name");
  template_scope.SetValue(target_name,
                          Value(invocation, args[0].string_value()),
                          invocation);

  // Actually run the template code.
  Value result =
      definition_->block()->ExecuteBlockInScope(&template_scope, err);
  if (err->has_error())
    return Value();

  // Check for unused variables in the invocation scope. This will find typos
  // of things the caller meant to pass to the template but the template didn't
  // read out.
  //
  // This is a bit tricky because it's theoretically possible for the template
  // to overwrite the value of "invoker" and free the Scope owned by the
  // value. So we need to look it up again and don't do anything if it doesn't
  // exist.
  invoker_value = template_scope.GetMutableValue(kInvoker, false);
  if (invoker_value && invoker_value->type() == Value::SCOPE) {
    if (!invoker_value->scope_value()->CheckForUnusedVars(err))
      return Value();
  }

  // Check for unused variables in the template itself.
  if (!template_scope.CheckForUnusedVars(err))
    return Value();

  return result;
}

LocationRange Template::GetDefinitionRange() const {
  return definition_->GetRange();
}
