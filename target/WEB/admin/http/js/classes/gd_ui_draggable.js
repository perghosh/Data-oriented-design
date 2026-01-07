/**
 * Class to make HTML elements draggable
 *
 * Dragging functionality for HTML elements. Meaning that the element can be moved around the page by clicking and dragging.
 *
 * This implementation is self-contained and doesn't require external CSS classes
 * or configurations.
 */
class UIDraggable {
   static iZIndex_s = 10; // Default z-index for draggable elements and used to stack elements

   /**
    * @param {HTMLElement|string} element_ - The element to make draggable
    * @param {Object} oOptions_ - Configuration options
    * @param {boolean} [oOptions_.bUseTransform=true] - Use transform property instead of position
    * @param {string} [oOptions_.sHandleSelector=null] - CSS selector for drag handle
    * @param {HTMLElement} [oOptions_.eHandle=null] - Element to use as drag handle
    * @param {Object} [oOptions_.oBounds=null] - Constrain dragging within bounds
    * @param {HTMLElement} [oOptions_.oBounds.eElement] - Element to constrain within
    * @param {Object} [oOptions_.oBounds.oPadding={top:0,right:0,bottom:0,left:0}] - Padding from bounds
    * @param {boolean} [oOptions_.bSnapToGrid=false] - Enable grid snapping
    * @param {number} [oOptions_.iGridSize=10] - Grid size for snapping
    * @param {Function} [oOptions_.fnOnDragStart] - Callback when dragging starts
    * @param {Function} [oOptions_.fnOnDragMove] - Callback during dragging
    * @param {Function} [oOptions_.fnOnDragEnd] - Callback when dragging ends
    */
   constructor(element_, oOptions_ = {}) {
      let eElement;
      if( typeof element_ === "string" ) {
         eElement = document.querySelector(element_);
         if( !eElement ) eElement = document.getElementById(element_);
      } else {
         eElement = element_;
      }

      if( !eElement ) { throw new Error('UIDraggable: Element not found'); }

      // Generate unique ID for element
      this.sId = `draggable-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
      this.eElement = eElement;

      // Apply options with defaults
      this.oOptions = Object.assign({
         bUseTransform: true,  // Use CSS transforms for positioning
         sHandleSelector: null,// Use element itself as handle
         eHandle: null,        // If set this element is used as drag handle
         oBounds: null,        // If set this element is used as bounds
         bSnapToGrid: false,   // Snap to grid
         iGridSize: 10,        // Grid size for snapping
         fnOnDragStart: null,  // Callback when drag starts
         fnOnDragMove: null,   // Callback when drag moves
         fnOnDragEnd: null     // Callback when drag ends
      }, oOptions_);

      // ## Initialize bounds padding if not provided
      if( this.oOptions.oBounds && !this.oOptions.oBounds.oPadding ) {
         this.oOptions.oBounds.oPadding = { top: 0, right: 0, bottom: 0, left: 0 };
      }

      // ## Initialize drag handle
      if( this.oOptions.eHandle ) {
         this.eDragHandle = this.oOptions.eHandle;
      }
      else {
         this.eDragHandle = this.oOptions.sHandleSelector ?
            this.eElement.querySelector(this.oOptions.sHandleSelector) :
            this.eElement;
      }

      // Prevent scrolling on touch devices and set grab cursor inline
      this.eDragHandle.style.touchAction = 'none';
      this.eDragHandle.style.cursor = 'grab';

      // Store original styles to restore on destroy
      this.oOriginalHandleStyle = {
         touchAction: this.eDragHandle.style.touchAction,
         cursor: this.eDragHandle.style.cursor
      };


      // ## We read the computed style to see if 'top' or 'left' are already set.
      const oComputed = window.getComputedStyle(this.eElement);
      const iInitialTop = parseFloat(oComputed.top) || 0;
      const iInitialLeft = parseFloat(oComputed.left) || 0;

      this.bDragging = false;
      this.iInitialX = 0;
      this.iInitialY = 0;

      // We set the current position to the CSS values so the first drag
      // starts from where the element actually is.
      this.iCurrentX = iInitialLeft;
      this.iCurrentY = iInitialTop;
      this.iXOffset = iInitialLeft;
      this.iYOffset = iInitialTop;

      // Store dragging styles for inline application
      this.oDraggingStyles = {
         cursor: 'grabbing',
         opacity: '0.8',
         position: 'relative',
         transition: 'none'
         //zIndex: '9999'
      };

      // Store original element styles for potential restoration
      this.oOriginalElementStyle = {
         cursor: this.eElement.style.cursor || '',
         opacity: this.eElement.style.opacity || '',
         position: this.eElement.style.position || oComputed.position,
         transition: this.eElement.style.transition || '',
         //zIndex: this.eElement.style.zIndex || '',
         top: this.eElement.style.top || '',
         left: this.eElement.style.left || ''
      };

      // Store bound handlers for proper removal
      this.oBoundHandlers = {
         down: this._on_pointer_down.bind(this),
         move: this._on_pointer_move.bind(this),
         up: this._on_pointer_up.bind(this)
      };

      this._initialize();

      // This prevents the element from "jumping" back to 0,0 if transforms are enabled
      if(this.oOptions.bUseTransform === true) { this._set_position(this.iCurrentX, this.iCurrentY); }
   }

   // ## Getter/Setter

   get id() { return this.sId; }
   set id(sId_) { this.sId = sId_; }
   get name() { return this.oOptions.sName; }
   set name(sName) { this.oOptions.sName = sName; }
   get dragging() { return this.bDragging; }
   set dragging(bDragging) {
      this.bDragging = bDragging;
      if(bDragging === true) {                                                // if true then increase z-index and set it
         UIDraggable.iZIndex_s++;
         this.eElement.style.zIndex = UIDraggable.iZIndex_s;
      }
   }

   /** -----------------------------------------------------------------------
    * Initialize event listeners
    */
   _initialize() {
      // Mouse events
      this.eDragHandle.addEventListener('mousedown', this.oBoundHandlers.down);
      document.addEventListener('mousemove', this.oBoundHandlers.move);
      document.addEventListener('mouseup', this.oBoundHandlers.up);

      // Touch events for mobile support
      this.eDragHandle.addEventListener('touchstart', this.oBoundHandlers.down, { passive: false });
      document.addEventListener('touchmove', this.oBoundHandlers.move, { passive: false });
      document.addEventListener('touchend', this.oBoundHandlers.up);
   }

   /** -----------------------------------------------------------------------
    * Handle pointer down (mouse/touch start)
    * @param {MouseEvent|TouchEvent} eEvent_ - The event object
    *
    * @TODO: Handle filtering based on what is pressed to not trigger the move operation
    */
   _on_pointer_down(eEvent_) {
      // Only handle if target is drag handle or a child of it
      const eTarget = eEvent_.target || eEvent_.touches?.[0]?.target;
      if( !this.eDragHandle.contains(eTarget) && this.eDragHandle !== eTarget ) {
         return;
      }

      // Prevent default for touch events to avoid scrolling
      if( eEvent_.cancelable ) {  eEvent_.preventDefault(); }

      // Get coordinates from mouse or touch
      const iClientX = eEvent_.clientX || eEvent_.touches?.[0]?.clientX;
      const iClientY = eEvent_.clientY || eEvent_.touches?.[0]?.clientY;

      // ## Store initial position
      this.iInitialX = iClientX - this.iXOffset;
      this.iInitialY = iClientY - this.iYOffset;
      this.dragging = true;

      // Apply dragging styles inline
      Object.assign(this.eElement.style, this.oDraggingStyles);
      this.eDragHandle.style.cursor = 'grabbing';

      // ## Trigger callback if provided
      if( this.oOptions.fnOnDragStart ) {
         this.oOptions.fnOnDragStart({
            eElement: this.eElement,
            iStartX: this.iInitialX,
            iStartY: this.iInitialY,
            iCurrentX: this.iCurrentX,
            iCurrentY: this.iCurrentY
         });
      }
   }

   /** -----------------------------------------------------------------------
    * Handle pointer move (mouse/touch move)
    * @param {MouseEvent|TouchEvent} eEvent_ - The event object
    */
   _on_pointer_move(eEvent_) {
      if( !this.dragging ) return;

      // Prevent default to avoid scrolling
      if( eEvent_.cancelable ) {
         eEvent_.preventDefault();
      }

      // ## Get coordinates from mouse or touch
      const iClientX = eEvent_.clientX || eEvent_.touches?.[0]?.clientX;
      const iClientY = eEvent_.clientY || eEvent_.touches?.[0]?.clientY;

      // ## Calculate new position
      let iNewX = iClientX - this.iInitialX;
      let iNewY = iClientY - this.iInitialY;

      // ## Apply grid snapping if enabled
      if( this.oOptions.bSnapToGrid ) {
         iNewX = Math.round(iNewX / this.oOptions.iGridSize) * this.oOptions.iGridSize;
         iNewY = Math.round(iNewY / this.oOptions.iGridSize) * this.oOptions.iGridSize;
      }

      // ## Apply bounds constraints if configured
      if( this.oOptions.oBounds ) {
         const oConstrained = this._apply_bounds(iNewX, iNewY);
         iNewX = oConstrained.iX;
         iNewY = oConstrained.iY;
      }

      // Update position
      this.iCurrentX = iNewX;
      this.iCurrentY = iNewY;
      this.iXOffset = iNewX;
      this.iYOffset = iNewY;

      this._set_position(iNewX, iNewY);

      // Trigger callback if provided
      if( this.oOptions.fnOnDragMove ) {
         this.oOptions.fnOnDragMove({
            eElement: this.eElement,
            iX: iNewX,
            iY: iNewY
         });
      }
   }

   /** -----------------------------------------------------------------------
    * Handle pointer up (mouse/touch end)
    */
   _on_pointer_up() {
      if( !this.dragging ) return;                                         // Return early if not dragging

      this.dragging = false;
      this.iInitialX = this.iCurrentX;                                        // Initialize initial position
      this.iInitialY = this.iCurrentY;                                        // Initialize initial position

      // ## before we apply them back. This prevents the "snap back". It should stay at the current position
      if(this.oOptions.bUseTransform) {
         this.oOriginalElementStyle.top = '0px';
         this.oOriginalElementStyle.left = '0px';
         this.oOriginalElementStyle.transform = `translate3d(${this.iCurrentX}px, ${this.iCurrentY}px, 0)`;
      }
      else {
         this.oOriginalElementStyle.top = `${this.iCurrentY}px`;
         this.oOriginalElementStyle.left = `${this.iCurrentX}px`;
      }

      // Restore original element styles
      Object.assign(this.eElement.style, this.oOriginalElementStyle);
      this.eDragHandle.style.cursor = 'grab';

      // Trigger callback if provided
      if( this.oOptions.fnOnDragEnd ) {
         this.oOptions.fnOnDragEnd({
            eElement: this.eElement,
            iX: this.iCurrentX,
            iY: this.iCurrentY
         });
      }
   }

   /** -----------------------------------------------------------------------
    * Apply bounds constraints to a position
    * @param {number} iX - X coordinate
    * @param {number} iY - Y coordinate
    * @returns {Object} Constrained position with iX and iY
    */
   _apply_bounds(iX, iY) {
      if( !this.oOptions.oBounds ) return { iX, iY };

      let oBounds = this.oOptions.oBounds;
      let oPadding = oBounds.oPadding;

      // If bounds element is specified, calculate bounds relative to that element
      if( oBounds.eElement ) {
         const oBoundsRect = oBounds.eElement.getBoundingClientRect();
         const oElementRect = this.eElement.getBoundingClientRect();

         const iWidth = oElementRect.width;
         const iHeight = oElementRect.height;

         const iMinX = oBoundsRect.left + oPadding.left;
         const iMaxX = oBoundsRect.right - iWidth - oPadding.right;
         const iMinY = oBoundsRect.top + oPadding.top;
         const iMaxY = oBoundsRect.bottom - iHeight - oPadding.bottom;

         // Convert to relative coordinates
         const oParentRect = this.eElement.parentElement.getBoundingClientRect();
         const iRelativeMinX = iMinX - oParentRect.left;
         const iRelativeMaxX = iMaxX - oParentRect.left;
         const iRelativeMinY = iMinY - oParentRect.top;
         const iRelativeMaxY = iMaxY - oParentRect.top;

         return {
            iX: Math.max(iRelativeMinX, Math.min(iRelativeMaxX, iX)),
            iY: Math.max(iRelativeMinY, Math.min(iRelativeMaxY, iY))
         };
      }

      // Default to viewport bounds
      return {
         iX: Math.max(0, Math.min(window.innerWidth - this.eElement.offsetWidth, iX)),
         iY: Math.max(0, Math.min(window.innerHeight - this.eElement.offsetHeight, iY))
      };
   }

   /** -----------------------------------------------------------------------
    * Set the element position
    * @param {number} iX - X position
    * @param {number} iY - Y position
    */
   _set_position(iX, iY) {
      const oStyle = this.eElement.style;
      if( this.oOptions.bUseTransform ) {
         // If we use transform, we should ideally set top/left to 0 so they don't
         // stack with the transform, OR we calculate the transform as a delta.
         // To keep it simple and clean, we set the actual styles to 0.
         oStyle.left = '0px';
         oStyle.top = '0px';
         oStyle.transform = `translate3d(${iX}px, ${iY}px, 0)`;
      }
      else {
         oStyle.left = `${iX}px`;
         oStyle.top = `${iY}px`;
      }
   }

   /** ------------------------------------------------------------------------
    * Get the current position of the element
    * @returns {Object} Position with x and y
    */
   GetPosition() {
      return { x: this.iCurrentX, y: this.iCurrentY };
   }

   /** ------------------------------------------------------------------------
    * Set the position of the element
    * @param {number} iX - X position
    * @param {number} iY - Y position
    */
   SetPosition(iX, iY) {
      this.iCurrentX = iX;
      this.iCurrentY = iY;
      this.iXOffset = iX;
      this.iYOffset = iY;
      this._set_position(iX, iY);
   }

   /**
    * Reset the element position to 0,0
    */
   ResetPosition() {
      this.iCurrentX = 0;
      this.iCurrentY = 0;
      this.iXOffset = 0;
      this.iYOffset = 0;
      this.SetPosition(0, 0);
   }

   /** -----------------------------------------------------------------------
    * Enable dragging
    */
   Enable() {
      if( !this.eDragHandle ) return;
      this.eDragHandle.style.pointerEvents = 'auto';
      this.eDragHandle.style.cursor = 'grab';
   }

   /** -----------------------------------------------------------------------
    * Disable dragging
    */
   Disable() {
      if( !this.eDragHandle ) return;
      this.eDragHandle.style.pointerEvents = 'none';
      this.eDragHandle.style.cursor = 'default';
   }

   /** -----------------------------------------------------------------------
    * Update configuration options
    * @param {Object} oNewOptions - New configuration options
    */
   UpdateOptions(oNewOptions) {
      this.oOptions = Object.assign({}, this.oOptions, oNewOptions);

      // Update drag handle if selector or element changed
      if( oNewOptions.sHandleSelector || oNewOptions.eHandle ) {
         // Restore previous handle style
         this.eDragHandle.style.touchAction = this.oOriginalHandleStyle.touchAction || '';
         this.eDragHandle.style.cursor = this.oOriginalHandleStyle.cursor || '';

         this.eDragHandle.removeEventListener('mousedown', this.oBoundHandlers.down);
         this.eDragHandle.removeEventListener('touchstart', this.oBoundHandlers.down);

         if( this.oOptions.eHandle ) {
            this.eDragHandle = this.oOptions.eHandle;
         }
         else {
            this.eDragHandle = this.oOptions.sHandleSelector ?
               this.eElement.querySelector(this.oOptions.sHandleSelector) :
               this.eElement;
         }

         // Apply new handle styles
         this.eDragHandle.style.touchAction = 'none';
         this.eDragHandle.style.cursor = 'grab';

         // Update stored original styles
         this.oOriginalHandleStyle = {
            touchAction: this.eDragHandle.style.touchAction,
            cursor: this.eDragHandle.style.cursor
         };

         this.eDragHandle.addEventListener('mousedown', this.oBoundHandlers.down);
         this.eDragHandle.addEventListener('touchstart', this.oBoundHandlers.down, { passive: false });
      }
   }

   /** -----------------------------------------------------------------------
    * Destroy the draggable instance and remove all event listeners
    */
   Destroy() {
      // Restore original styles
      Object.assign(this.eElement.style, this.oOriginalElementStyle);
      if( this.eDragHandle ) {
         Object.assign(this.eDragHandle.style, this.oOriginalHandleStyle);
      }

      // Remove event listeners
      if( this.eDragHandle ) {
         this.eDragHandle.removeEventListener('mousedown', this.oBoundHandlers.down);
         this.eDragHandle.removeEventListener('touchstart', this.oBoundHandlers.down);
      }

      document.removeEventListener('mousemove', this.oBoundHandlers.move);
      document.removeEventListener('mouseup', this.oBoundHandlers.up);
      document.removeEventListener('touchmove', this.oBoundHandlers.move);
      document.removeEventListener('touchend', this.oBoundHandlers.up);

      // Clear references
      this.eElement = null;
      this.eDragHandle = null;
      this.oBoundHandlers = null;
   }
}
