// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// require: event_target.js

cr.define('cr.ui', function() {
  /** @const */ var EventTarget = cr.EventTarget;
  /** @const */ var Event = cr.Event;
  /** @const */ var Menu = cr.ui.Menu;

  /**
   * Handles context menus.
   * @constructor
   * @extends {EventTarget}
   */
  function ContextMenuHandler() {}

  ContextMenuHandler.prototype = {
    __proto__: EventTarget.prototype,

    /**
     * The menu that we are currently showing.
     * @type {cr.ui.Menu}
     */
    menu_: null,
    get menu() {
      return this.menu_;
    },

    /**
     * Shows a menu as a context menu.
     * @param {!Event} e The event triggering the show (usually a contextmenu
     *     event).
     * @param {!cr.ui.Menu} menu The menu to show.
     */
    showMenu: function(e, menu) {
      menu.updateCommands(e.currentTarget);
      if (!menu.hasVisibleItems())
        return;

      this.menu_ = menu;
      menu.classList.remove('hide-delayed');
      menu.hidden = false;
      menu.contextElement = e.currentTarget;

      // when the menu is shown we steal all keyboard events.
      var doc = menu.ownerDocument;
      doc.addEventListener('keydown', this, true);
      doc.addEventListener('mousedown', this, true);
      doc.addEventListener('focus', this);
      doc.defaultView.addEventListener('resize', this);
      menu.addEventListener('contextmenu', this);
      menu.addEventListener('activate', this);
      this.positionMenu_(e, menu);

      var ev = new Event('show');
      ev.element = menu.contextElement;
      ev.menu = menu;
      this.dispatchEvent(ev);
    },

    /**
     * Hide the currently shown menu.
     * @param {HideType=} opt_hideType Type of hide.
     *     default: cr.ui.HideType.INSTANT.
     */
    hideMenu: function(opt_hideType) {
      var menu = this.menu;
      if (!menu)
        return;

      if (opt_hideType == cr.ui.HideType.DELAYED)
        menu.classList.add('hide-delayed');
      else
        menu.classList.remove('hide-delayed');
      menu.hidden = true;
      var originalContextElement = menu.contextElement;
      menu.contextElement = null;
      var doc = menu.ownerDocument;
      doc.removeEventListener('keydown', this, true);
      doc.removeEventListener('mousedown', this, true);
      doc.removeEventListener('focus', this);
      doc.defaultView.removeEventListener('resize', this);
      menu.removeEventListener('contextmenu', this);
      menu.removeEventListener('activate', this);
      menu.selectedIndex = -1;
      this.menu_ = null;

      // On windows we might hide the menu in a right mouse button up and if
      // that is the case we wait some short period before we allow the menu
      // to be shown again.
      this.hideTimestamp_ = cr.isWindows ? Date.now() : 0;

      var ev = new Event('hide');
      ev.element = menu.contextElement;
      ev.menu = menu;
      this.dispatchEvent(ev);
    },

    /**
     * Positions the menu
     * @param {!Event} e The event object triggering the showing.
     * @param {!cr.ui.Menu} menu The menu to position.
     * @private
     */
    positionMenu_: function(e, menu) {
      // TODO(arv): Handle scrolled documents when needed.

      var element = e.currentTarget;
      var x, y;
      // When the user presses the context menu key (on the keyboard) we need
      // to detect this.
      if (this.keyIsDown_) {
        var rect = element.getRectForContextMenu ?
                       element.getRectForContextMenu() :
                       element.getBoundingClientRect();
        var offset = Math.min(rect.width, rect.height) / 2;
        x = rect.left + offset;
        y = rect.top + offset;
      } else {
        x = e.clientX;
        y = e.clientY;
      }

      cr.ui.positionPopupAtPoint(x, y, menu);
    },

    /**
     * Handles event callbacks.
     * @param {!Event} e The event object.
     */
    handleEvent: function(e) {
      // Keep track of keydown state so that we can use that to determine the
      // reason for the contextmenu event.
      switch (e.type) {
        case 'keydown':
          this.keyIsDown_ = !e.ctrlKey && !e.altKey &&
              // context menu key or Shift-F10
              (e.keyCode == 93 && !e.shiftKey ||
               e.keyIdentifier == 'F10' && e.shiftKey);
          break;

        case 'keyup':
          this.keyIsDown_ = false;
          break;
      }

      // Context menu is handled even when we have no menu.
      if (e.type != 'contextmenu' && !this.menu)
        return;

      switch (e.type) {
        case 'mousedown':
          if (!this.menu.contains(e.target))
            this.hideMenu();
          else
            e.preventDefault();
          break;
        case 'keydown':
          // keyIdentifier does not report 'Esc' correctly
          if (e.keyCode == 27 /* Esc */) {
            this.hideMenu();
            e.stopPropagation();
            e.preventDefault();

          // If the menu is visible we let it handle all the keyboard events.
          } else if (this.menu) {
            this.menu.handleKeyDown(e);
            e.preventDefault();
            e.stopPropagation();
          }
          break;

        case 'activate':
          var hideDelayed = e.target instanceof cr.ui.MenuItem &&
              e.target.checkable;
          this.hideMenu(hideDelayed ? cr.ui.HideType.DELAYED :
                                      cr.ui.HideType.INSTANT);
          break;

        case 'focus':
          if (!this.menu.contains(e.target))
            this.hideMenu();
          break;

        case 'resize':
          this.hideMenu();
          break;

        case 'contextmenu':
          if ((!this.menu || !this.menu.contains(e.target)) &&
              (!this.hideTimestamp_ || Date.now() - this.hideTimestamp_ > 50))
            this.showMenu(e, e.currentTarget.contextMenu);
          e.preventDefault();
          // Don't allow elements further up in the DOM to show their menus.
          e.stopPropagation();
          break;
      }
    },

    /**
     * Adds a contextMenu property to an element or element class.
     * @param {!Element|!Function} element The element or class to add the
     *     contextMenu property to.
     */
    addContextMenuProperty: function(element) {
      if (typeof element == 'function')
        element = element.prototype;

      element.__defineGetter__('contextMenu', function() {
        return this.contextMenu_;
      });
      element.__defineSetter__('contextMenu', function(menu) {
        var oldContextMenu = this.contextMenu;

        if (typeof menu == 'string' && menu[0] == '#') {
          menu = this.ownerDocument.getElementById(menu.slice(1));
          cr.ui.decorate(menu, Menu);
        }

        if (menu === oldContextMenu)
          return;

        if (oldContextMenu && !menu) {
          this.removeEventListener('contextmenu', contextMenuHandler);
          this.removeEventListener('keydown', contextMenuHandler);
          this.removeEventListener('keyup', contextMenuHandler);
        }
        if (menu && !oldContextMenu) {
          this.addEventListener('contextmenu', contextMenuHandler);
          this.addEventListener('keydown', contextMenuHandler);
          this.addEventListener('keyup', contextMenuHandler);
        }

        this.contextMenu_ = menu;

        if (menu && menu.id)
          this.setAttribute('contextmenu', '#' + menu.id);

        cr.dispatchPropertyChange(this, 'contextMenu', menu, oldContextMenu);
      });

      if (!element.getRectForContextMenu) {
        /**
         * @return {!ClientRect} The rect to use for positioning the context
         *     menu when the context menu is not opened using a mouse position.
         */
        element.getRectForContextMenu = function() {
          return this.getBoundingClientRect();
        };
      }
    },

    /**
     * Sets the given contextMenu to the given element. A contextMenu property
     * would be added if necessary.
     * @param {!Element} element The element or class to set the contextMenu to.
     * @param {!cr.ui.Menu} contextMenu The contextMenu property to be set.
     */
    setContextMenu: function(element, contextMenu) {
      if (!element.contextMenu)
        this.addContextMenuProperty(element);
      element.contextMenu = contextMenu;
    }
  };

  /**
   * The singleton context menu handler.
   * @type {!ContextMenuHandler}
   */
  var contextMenuHandler = new ContextMenuHandler;

  // Export
  return {
    contextMenuHandler: contextMenuHandler,
  };
});
