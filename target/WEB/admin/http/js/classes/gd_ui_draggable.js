/**
 * Class to make HTML elements draggable
 *
 * Dragging functionality for HTML elements. Meaning that the element can be moved around the page by clicking and dragging.
 *
 * This implementation is self-contained and doesn't require external CSS classes
 * or configurations.
 */
class UIDraggable {
   /**
    * @param {HTMLElement|string} eElement_ - The element to make draggable
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
   constructor(eElement_, oOptions_ = {}) {
      let eElement;
      if( typeof eElement_ === "string" ) {
         eElement = document.querySelector(eElement_);
         if( !eElement ) eElement = document.getElementById(eElement_);
      } else {
         eElement = eElement_;
      }

      if( !eElement ) { throw new Error('UIDraggable: Element not found'); }

      // Generate unique ID for element
      this.sId = `draggable-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
      this.eElement = eElement;

      // Apply options with defaults
      this.oOptions = Object.assign({
         bUseTransform: true,
         sHandleSelector: null,
         eHandle: null,
         oBounds: null,
         bSnapToGrid: false,
         iGridSize: 10,
         fnOnDragStart: null,
         fnOnDragMove: null,
         fnOnDragEnd: null
      }, oOptions_);

      // Initialize bounds padding if not provided
      if( this.oOptions.oBounds && !this.oOptions.oBounds.oPadding ) {
         this.oOptions.oBounds.oPadding = { top: 0, right: 0, bottom: 0, left: 0 };
      }

      // Initialize drag handle
      if( this.oOptions.eHandle ) {
         this.eDragHandle = this.oOptions.eHandle;
      }
      else {
         this.eDragHandle = this.oOptions.sHandleSelector ?
            this.eElement.querySelector(this.oOptions.sHandleSelector) :
            this.eElement;
      }

      // Prevent scrolling on touch devices
      this.eDragHandle.style.touchAction = 'none';

      // Initialize state
      this.bIsDragging = false;
      this.iInitialX = 0;
      this.iInitialY = 0;
      this.iCurrentX = 0;
      this.iCurrentY = 0;
      this.iXOffset = 0;
      this.iYOffset = 0;

      // Create inline styles for dragging state (self-contained, no external CSS)
      this._create_drag_styles();

      // Store bound handlers for proper removal
      this.oBoundHandlers = {
         down: this._on_pointer_down.bind(this),
         move: this._on_pointer_move.bind(this),
         up: this._on_pointer_up.bind(this)
      };

      this._initialize();
   }

   // Getter/setter for id
   get id() { return this.sId; }
   set id(sId_) { this.sId = sId_; }

   /** -----------------------------------------------------------------------
    * Create inline styles for dragging state (no external CSS dependency)
    */
   _create_drag_styles() {
      // Create a style element if it doesn't exist
      if( !document.getElementById('gd-draggable-styles') ) {
         const eStyle = document.createElement('style');
         eStyle.id = 'gd-draggable-styles';
         eStyle.textContent = `
            .gd-draggable-dragging { cursor: grabbing !important; opacity: 0.8 !important; position: relative !important; transition: none !important;  z-index: 9999 !important; }
            .gd-draggable-handle { cursor: grab !important; }
         `;
         document.head.appendChild(eStyle);
      }

      // Add handle class to drag handle
      this.eDragHandle.classList.add('gd-draggable-handle');
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
      this.bIsDragging = true;

      // Add visual feedback
      this.eElement.classList.add('gd-draggable-dragging');

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
      if( !this.bIsDragging ) return;

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

      _set_position(iNewX, iNewY);

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
      if( !this.bIsDragging ) return;

      this.bIsDragging = false;
      this.iInitialX = this.iCurrentX;
      this.iInitialY = this.iCurrentY;

      // Remove visual feedback
      this.eElement.classList.remove('gd-draggable-dragging');

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

         const iMinX = oBoundsRect.left + oPadding.left;
         const iMaxX = oBoundsRect.right - oElementRect.width - oPadding.right;
         const iMinY = oBoundsRect.top + oPadding.top;
         const iMaxY = oBoundsRect.bottom - oElementRect.height - oPadding.bottom;

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
         oStyle.transform = `translate3d(${iX}px, ${iY}px, 0)`;
      }
      else {
         oStyle.position = 'absolute';
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
      this._set_position(iNewX, iNewY);
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
   }

   /** -----------------------------------------------------------------------
    * Disable dragging
    */
   Disable() {
      if( !this.eDragHandle ) return;
      this.eDragHandle.style.pointerEvents = 'none';
   }

   /** -----------------------------------------------------------------------
    * Update configuration options
    * @param {Object} oNewOptions - New configuration options
    */
   UpdateOptions(oNewOptions) {
      this.oOptions = Object.assign({}, this.oOptions, oNewOptions);

      // Update drag handle if selector or element changed
      if( oNewOptions.sHandleSelector || oNewOptions.eHandle ) {
         this.eDragHandle.style.touchAction = '';
         this.eDragHandle.classList.remove('gd-draggable-handle');
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

         this.eDragHandle.style.touchAction = 'none';
         this.eDragHandle.classList.add('gd-draggable-handle');
         this.eDragHandle.addEventListener('mousedown', this.oBoundHandlers.down);
         this.eDragHandle.addEventListener('touchstart', this.oBoundHandlers.down, { passive: false });
      }
   }

   /** -----------------------------------------------------------------------
    * Destroy the draggable instance and remove all event listeners
    */
   Destroy() {
      // Remove visual feedback
      this.eElement.classList.remove('gd-draggable-dragging');
      this.eDragHandle.classList.remove('gd-draggable-handle');

      // Remove event listeners
      if( this.eDragHandle ) {
         this.eDragHandle.removeEventListener('mousedown', this.oBoundHandlers.down);
         this.eDragHandle.removeEventListener('touchstart', this.oBoundHandlers.down);
         this.eDragHandle.style.touchAction = '';
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
