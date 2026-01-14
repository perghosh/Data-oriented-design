/**
 * Class to create a Tab Control
 *
 * A self-contained tab control that handles tab switching and content display.
 * Tabs can be positioned at the top or bottom of the content area.
 *
 * This implementation is self-contained and doesn't require external CSS classes
 * or configurations.
 */
class UITabControl {
   static iCount_s = 0; // Static counter for unique IDs

   /**
    * @param {HTMLElement|string} element_ - The container element for the tab control
    * @param {Object} oOptions_ - Configuration options
    * @param {string} [oOptions_.sPosition="top"] - Tab position: "top" or "bottom"
    * @param {string} [oOptions_.sActiveClass="active"] - CSS class name for the active tab and panel
    * @param {Array<Object>} [oOptions_.aTabs=[]] - Initial tabs to add [{ sTitle, eContent }, ...]
    */
   constructor(element_, oOptions_ = {}) {
      let eElement;
      if( typeof element_ === "string" ) {
         eElement = document.querySelector(element_);
         if( !eElement ) eElement = document.getElementById(element_);
      }
      else {
         eElement = element_;
      }

      if( !eElement ) { throw new Error('UITabControl: Element not found'); }

      // Generate unique ID for the control
      this.sId = `tab-control-${UITabControl.iCount_s++}`;
      this.eElement = eElement;

      // Apply options with defaults
      this.oOptions = Object.assign({
         sPosition: 'top',     // Position of tabs: 'top' or 'bottom'
         sActiveClass: 'active', // CSS class for active state
         aTabs: []             // Initial tab definitions
      }, oOptions_);

      // Store internal state
      this.aTabs = [];         // Array to store tab objects: { eButton, ePanel, sTitle, eContent }
      this.iActiveIndex = -1;  // Index of the currently active tab

      // Create structural elements
      this.eHeader = document.createElement('div'); // Container for tab buttons
      this.eBody = document.createElement('div');   // Container for tab panels

      // Store bound handlers for proper removal
      this.oBoundHandlers = {
         click: this._on_click.bind(this)
      };

      this._initialize();
      this._apply_styles();

      // ## Add initial tabs if provided .....................................
      if( Array.isArray(this.oOptions.aTabs) && this.oOptions.aTabs.length > 0 ) {
         this.oOptions.aTabs.forEach(oTab => {
            if( oTab.sTitle && oTab.eContent ) {
               this.AddTab(oTab.sTitle, oTab.eContent);
            }
         });
      }
   }

   /** -----------------------------------------------------------------------
    * Initialize the DOM structure and event listeners
    */
   _initialize() {
      // Clear the container element
      this.eElement.innerHTML = '';

      // Set internal classes for identification (helper for inline styles)
      this.eHeader.className = 'ui-tab-header';
      this.eBody.className = 'ui-tab-body';

      // Append elements based on position configuration
      if( this.oOptions.sPosition === 'bottom' ) {
         this.eElement.appendChild(this.eBody);
         this.eElement.appendChild(this.eHeader);
      } else {
         // Default to top
         this.eElement.appendChild(this.eHeader);
         this.eElement.appendChild(this.eBody);
      }

      // Use event delegation on the header to handle tab clicks
      this.eHeader.addEventListener('click', this.oBoundHandlers.click);
   }

   /** -----------------------------------------------------------------------
    * Apply default inline styles to the structure to ensure it works without CSS
    */
   _apply_styles() {
      // Container styles
      const sFlexDir = this.oOptions.sPosition === 'bottom' ? 'column-reverse' : 'column';
      Object.assign(this.eElement.style, {
         display: 'flex',
         flexDirection: sFlexDir,
         width: '100%',
         boxSizing: 'border-box',
         overflow: 'hidden' // Prevent overflow from content
      });

      // Header (Tab list) styles
      Object.assign(this.eHeader.style, {
         display: 'flex',
         flexDirection: 'row',
         backgroundColor: '#f0f0f0',
         borderBottom: this.oOptions.sPosition === 'top' ? '1px solid #ccc' : 'none',
         borderTop: this.oOptions.sPosition === 'bottom' ? '1px solid #ccc' : 'none',
         padding: '0',
         margin: '0',
         listStyle: 'none',
         overflowX: 'auto',
         whiteSpace: 'nowrap'
      });

      // Body (Panels) styles
      Object.assign(this.eBody.style, {
         flex: '1',
         position: 'relative',
         overflow: 'auto',
         backgroundColor: '#fff'
      });
   }

   /** -----------------------------------------------------------------------
    * Handle click events on the tab header (Event Delegation)
    * @param {MouseEvent} eEvent_
    */
   _on_click(eEvent_) {
      // Traverse up to find the closest tab button
      const eTarget = eEvent_.target.closest('[data-tab-index]');

      // If we found a button and it belongs to this header
      if( eTarget && this.eHeader.contains(eTarget) ) {
         const iIndex = parseInt(eTarget.getAttribute('data-tab-index'), 10);
         this.SetActiveTab(iIndex);
      }
   }

   /** -----------------------------------------------------------------------
    * Helper to normalize content input (Element, Fragment, or Selector)
    * @param {HTMLElement|DocumentFragment|string} content_
    * @returns {HTMLElement|DocumentFragment}
    */
   _normalize_content(content_) {
      if (typeof content_ === 'string') {
         // Regular expression to check if the string starts with '<' and ends with '>'
         const bIsHTML = /<[a-z][\s\S]*>/i.test(content_);

         if (bIsHTML) {
            // It's raw HTML: Create a container and inject it
            const eTemp = document.createElement('div');
            eTemp.innerHTML = content_;
            // Return the first child or the container itself
            return eTemp.firstElementChild || eTemp;
         }
         else {
            // It's a selector: Try to find the element
            const eFound = document.querySelector(content_);
            // If found, move it; if not, create an empty div
            return eFound ? eFound : document.createElement('div');
         }
      }
      return content_;
   }

   /** -----------------------------------------------------------------------
    * Add a new tab to the control
    * @param {string} sTitle - The title text for the tab button
    * @param {HTMLElement|DocumentFragment|string} eContent_ - The content for the panel
    * @returns {number} The index of the added tab
    */
   AddTab(sTitle, eContent_) {
      const iIndex = this.aTabs.length;
      const eContent = this._normalize_content(eContent_);

      // ## Create Tab Button ...............................................
      const eButton = document.createElement('button');
      eButton.textContent = sTitle;
      eButton.setAttribute('data-tab-index', iIndex);
      eButton.setAttribute('type', 'button');

      // Default button styles
      Object.assign(eButton.style, {
         padding: '10px 20px',
         border: '1px solid #ccc',
         borderBottom: this.oOptions.sPosition === 'top' ? 'none' : '1px solid #ccc',
         borderTop: this.oOptions.sPosition === 'bottom' ? 'none' : '1px solid #ccc',
         backgroundColor: '#e0e0e0',
         cursor: 'pointer',
         marginRight: '2px',
         outline: 'none',
         fontSize: '14px'
      });

      // ## Create Tab Panel ................................................
      const ePanel = document.createElement('div');
      ePanel.appendChild(eContent); // Move content into panel

      // Default panel styles
      Object.assign(ePanel.style, {
         display: 'none', // Hidden by default
         //padding: '20px',
         width: '100%',
         height: '100%',
         boxSizing: 'border-box'
      });

      // Append to DOM
      this.eHeader.appendChild(eButton);
      this.eBody.appendChild(ePanel);

      // Store reference
      this.aTabs.push({
         sTitle: sTitle,
         eButton: eButton,
         ePanel: ePanel
      });

      // Activate if it's the first tab
      if( this.aTabs.length === 1 ) {
         this.SetActiveTab(0);
      }

      return iIndex;
   }

   /** -----------------------------------------------------------------------
    * Remove a tab by index
    * @param {number} iIndex - Index of the tab to remove
    */
   RemoveTab(iIndex) {
      if( iIndex < 0 || iIndex >= this.aTabs.length ) return;

      const oTab = this.aTabs[iIndex];

      // Remove DOM elements
      if( oTab.eButton && oTab.eButton.parentNode ) {
         oTab.eButton.parentNode.removeChild(oTab.eButton);
      }
      if( oTab.ePanel && oTab.ePanel.parentNode ) {
         oTab.ePanel.parentNode.removeChild(oTab.ePanel);
      }

      this.aTabs.splice(iIndex, 1);                                            // Remove from array

      // Update data-tab-index attributes for subsequent tabs
      this.aTabs.forEach((oTab, iNewIndex) => {
         oTab.eButton.setAttribute('data-tab-index', iNewIndex);
      });

      // Handle active tab reset
      if( this.iActiveIndex === iIndex ) {
         if( this.aTabs.length > 0 ) {
            // Try to activate the same index, or the previous one
            const iNextActive = Math.min(iIndex, this.aTabs.length - 1);
            this.SetActiveTab(iNextActive);
         }
         else {
            this.iActiveIndex = -1;
         }
      } else if( this.iActiveIndex > iIndex ) {
         // Shift active index down if we removed a tab before it
         this.iActiveIndex--;
      }
   }

   /** -----------------------------------------------------------------------
    * Set the active tab by index
    * @param {number} iIndex - Index of the tab to activate
    */
   SetActiveTab(iIndex) {
      if( iIndex < 0 || iIndex >= this.aTabs.length ) return;

      // Deactivate current tab
      if( this.iActiveIndex !== -1 && this.aTabs[this.iActiveIndex] ) {
         const oOldTab = this.aTabs[this.iActiveIndex];
         oOldTab.eButton.classList.remove(this.oOptions.sActiveClass);
         oOldTab.ePanel.classList.remove(this.oOptions.sActiveClass);

         // Revert inline styles for inactive state
         oOldTab.eButton.style.backgroundColor = '#e0e0e0';
         oOldTab.ePanel.style.display = 'none';
      }

      // Activate new tab
      this.iActiveIndex = iIndex;
      const oNewTab = this.aTabs[iIndex];
      oNewTab.eButton.classList.add(this.oOptions.sActiveClass);
      oNewTab.ePanel.classList.add(this.oOptions.sActiveClass);

      // Apply active inline styles
      oNewTab.eButton.style.backgroundColor = 'transparent';
      oNewTab.ePanel.style.display = 'block';
   }

   /** -----------------------------------------------------------------------
    * Get the index of the currently active tab
    * @returns {number} Active tab index
    */
   GetActiveTab() {
      return this.iActiveIndex;
   }

   /** -----------------------------------------------------------------------
    * Get tab object by index
    * @param {number} iIndex
    * @returns {Object|null} Tab object { sTitle, eButton, ePanel }
    */
   GetTab(iIndex) {
      if( iIndex < 0 || iIndex >= this.aTabs.length ) return null;
      return this.aTabs[iIndex];
   }

   /** -----------------------------------------------------------------------
    * Destroy the control and clean up event listeners
    */
   Destroy() {
      this.eHeader.removeEventListener('click', this.oBoundHandlers.click);
      this.eElement.innerHTML = ''; // Clear content

      this.aTabs = [];
      this.eElement = null;
      this.eHeader = null;
      this.eBody = null;
   }
}
