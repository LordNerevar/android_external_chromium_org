/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WebStorageAreaImpl.h"

#if ENABLE(DOM_STORAGE)

#include "ExceptionCode.h"
#include "SecurityOrigin.h"
#include "WebString.h"

namespace WebKit {

WebStorageAreaImpl::WebStorageAreaImpl(PassRefPtr<WebCore::StorageArea> storageArea, PassRefPtr<WebCore::SecurityOrigin> origin)
    : m_storageArea(storageArea)
    , m_origin(origin)
{
}

WebStorageAreaImpl::~WebStorageAreaImpl()
{
}

void WebStorageAreaImpl::lock(bool& invalidateCache, size_t& bytesLeftInQuota)
{
    // FIXME: Enable locking.  http://crbug.com/16877
    invalidateCache = false;
    // FIXME: Enable quota support.  http://crbug.com/16876
    bytesLeftInQuota = 0;
}

void WebStorageAreaImpl::unlock()
{
    // FIXME: Enable locking.  http://crbug.com/16877
}

unsigned WebStorageAreaImpl::length()
{
    return m_storageArea->length();
}

WebString WebStorageAreaImpl::key(unsigned index, bool& keyException)
{
    int exceptionCode = 0;
    WebString value = m_storageArea->key(index, exceptionCode);
    if (exceptionCode != 0) {
        ASSERT(exceptionCode == WebCore::INDEX_SIZE_ERR);
        keyException = true;
    } else {
        keyException = false;
    }
    return value;
}

WebString WebStorageAreaImpl::getItem(const WebString& key)
{
    return m_storageArea->getItem(key);
}

void WebStorageAreaImpl::setItem(const WebString& key, const WebString& value, bool& quotaException)
{
    int exceptionCode = 0;
    // FIXME: Can we do any better than just passing 0 for the frame?
    m_storageArea->setItem(key, value, exceptionCode, 0);
    if (exceptionCode != 0) {
        ASSERT(exceptionCode == WebCore::QUOTA_EXCEEDED_ERR);
        quotaException = true;
    } else {
        quotaException = false;
    }
}

void WebStorageAreaImpl::removeItem(const WebString& key)
{
    // FIXME: Can we do any better than just passing 0 for the frame?
    m_storageArea->removeItem(key, 0);
}

void WebStorageAreaImpl::clear()
{
    // FIXME: Can we do any better than just passing 0 for the frame?
    m_storageArea->clear(0);
}

} // namespace WebKit

#endif // ENABLE(DOM_STORAGE)
