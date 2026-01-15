/**
 * Class to create a Tab Control (Logic Only).
 *
 * This class manages the DOM structure and state (switching tabs) but applies
 * ZERO internal styling. The caller is responsible for all CSS.
 *
 * ## Required CSS Implementation
 * Since this class does not set `display: none/block`, you must define
 * visibility rules in your CSS using the classes defined in the `style` object.
 *
 * 1. **Layout**:
 *    - The container element (`element_`) has no default display property.
 *    - You should apply a layout (e.g., `display: flex; flex-direction: column;`) to the container
 *      or define layout properties in `class_header` and `class_body` to ensure
 *      the buttons and panels arrange correctly.
 *
 * 2. **Tab Visibility**:
 *    - **Panel**: You must define CSS for `style.class_panel` to hide the content by default
 *      (e.g., `display: none;`).
 *    - **Active Panel**: You must define CSS for `style.class_active` to show the content
 *      (e.g., `display: block;`). This class is added to the panel when active.
 *
 * 3. **Button State**:
 *    - **Button**: Define standard styles in `style.class_button`.
 *    - **Active Button**: Define styles for `style.class_button_active` (e.g., different background
 *      color or border) to indicate which tab is selected.
 *
 * @param {HTMLElement|string} element_ - The container element for the tab control.
 * @param {Object} [options_={}] - Configuration options.
 * @param {string} [options_.sPosition="top"] - Tab position: "top" or "bottom".
 *                                              Determines DOM order (Header before or after Body).
 * @param {Object} [options_.style] - Object defining CSS class names for elements.
 * @param {string} [options_.style.class_active] - Class applied to the currently active panel.
 * @param {string} [options_.style.class_header] - Class for the header container (holds buttons).
 * @param {string} [options_.style.class_body] - Class for the body container (holds panels).
 * @param {string} [options_.style.class_button] - Base class for all tab buttons.
 * @param {string} [options_.style.class_button_active] - Class applied to the currently active button.
 * @param {string} [options_.style.class_panel] - Class for the panel wrapper.
 *                                                **IMPORTANT:** Set `display: none` in CSS for this class.
 * @param {string} [options_.style.class_content] - Class applied to the content element inside the panel.
 * @param {Array<Object>} [options_.aTabs=[]] - Initial tabs to add: `[{ sTitle, eContent }, ...]`.
 */
class UITabControl {
   static iCount_s = 0; // Static counter for unique IDs

   constructor(element_, options_ = {}) {
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
      // Apply options with defaults
      this.oOptions = Object.assign({
         sPosition: 'top',     // Position of tabs: 'top' or 'bottom'
         style: {
            class_active: null,
            class_button: null,
            class_button_active: null,
            class_panel: null,
            class_content: null,
            class_header: null,
            class_body: null
         },
         aTabs: []             // Initial tab definitions
      }, options_);

      this.oStyle = this.oOptions.style; // styling

      // Store internal state
      this.aTabs = [];         // Array to store tab objects: { eButton, ePanel, sTitle, eContent }
      this.iActiveIndex = -1;  // Index of the currently active tab

      // Create structural elements
      this.eHeader = document.createElement('div'); // Container for tab buttons
      this.eBody = document.createElement('div');   // Container for tab panels

      // Store bound handlers for proper removal
      this.oBoundHandlers = { click: this._on_click.bind(this) };

      this._initialize();

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
    * Initialize the DOM structure.
    * Sets DOM order based on position, applies user classes only.
    */
   _initialize() {
      this.eElement.innerHTML = '';

      // Apply ONLY user-defined classes
      if( this.oStyle.class_header ) this.eHeader.classList.add(this.oStyle.class_header);
      if( this.oStyle.class_body ) this.eBody.classList.add(this.oStyle.class_body);

      // Determine DOM order based on position (Top vs Bottom)
      // Note: No CSS classes are added for position; we rely on DOM tree order.
      if( this.oOptions.sPosition === 'bottom' ) {
         this.eElement.appendChild(this.eBody);
         this.eElement.appendChild(this.eHeader);
      } else {
         // Default to top
         this.eElement.appendChild(this.eHeader);
         this.eElement.appendChild(this.eBody);
      }

      this.eHeader.addEventListener('click', this.oBoundHandlers.click);
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
      if(typeof content_ === 'string') {
         // Regular expression to check if the string starts with '<' and ends with '>'
         const bIsHTML = /<[a-z][\s\S]*>/i.test(content_);

         if(bIsHTML) {
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
    * Set the active tab by index
    * Toggles classes for styling but sets display property inline.
    *
    * @param {number} iIndex - The index of the tab to activate.
    */
   SetActiveTab(iIndex) {
      if( iIndex < 0 || iIndex >= this.aTabs.length ) return;

      const sClassActive = this.oStyle.class_button_active;
      const sClassNormal = this.oStyle.class_button;
      const sPanelActiveClass = this.oStyle.class_active;

      // ## Deactivate current tab ...........................................
      if( this.iActiveIndex !== -1 && this.aTabs[this.iActiveIndex] ) {
         const oOldTab = this.aTabs[this.iActiveIndex];

         // ### Toggle Button Classes ........................................
         if( sClassActive ) oOldTab.eButton.classList.remove(sClassActive);
         if( sClassNormal ) oOldTab.eButton.classList.add(sClassNormal);

         oOldTab.ePanel.classList.remove(sPanelActiveClass);                  // Toggle Panel Class

         oOldTab.ePanel.style.display = 'none';                               // Hardcode display style to hide the panel
      }

      this.iActiveIndex = iIndex;                                             // Activate new tab
      const oNewTab = this.aTabs[iIndex];

      // Toggle Button Classes
      if( sClassNormal ) { oNewTab.eButton.classList.remove(sClassNormal); }
      if( sClassActive ) { oNewTab.eButton.classList.add(sClassActive); }

      oNewTab.ePanel.classList.add(sPanelActiveClass);                        // Toggle Panel Class

      oNewTab.ePanel.style.display = 'block';                                 // Hardcode display style to show the panel
   }

   /** -----------------------------------------------------------------------
    * Add a new tab
    * @param {string} sTitle - The title text for the tab button
    * @param {HTMLElement|DocumentFragment|string} content_ - The content for the panel
    * @returns {number} The index of the added tab
    */
   AddTab(sTitle, content_) {
      const iIndex = this.aTabs.length;                                       // Get next tab index
      const eContent = this._normalize_content(content_);                     // Normalize content input

      // ## Create Tab Button ................................................
      const eButton = document.createElement('button');                       // Create button element
      eButton.textContent = sTitle;                                           // Set button title
      eButton.setAttribute('data-tab-index', iIndex);                         // Store tab index
      eButton.setAttribute('type', 'button');                                 // Set button type

      if( this.oStyle.class_button ) { eButton.classList.add(this.oStyle.class_button); }

      // ## Create Tab Panel ................................................
      const ePanel = document.createElement('div');                           // Create panel element
      ePanel.appendChild(eContent);                                           // Append content to panel

      if (this.oStyle.class_panel) { ePanel.classList.add(this.oStyle.class_panel); }
      ePanel.style.display = 'none';

      if( this.oStyle.class_content && eContent instanceof HTMLElement ) { eContent.classList.add(this.oStyle.class_content); } // Apply External Content Class

      // ## Append to DOM and Store Reference ...............................
      this.eHeader.appendChild(eButton);                                      // Append button to header
      this.eBody.appendChild(ePanel);                                         // Append panel to body

      this.aTabs.push({                                                       // Store reference
         sTitle: sTitle,
         eButton: eButton,
         ePanel: ePanel
      });

      if( this.aTabs.length === 1 ) { this.SetActiveTab(0); }                 // Activate first tab

      return iIndex;                                                          // Return tab index
   }


   /** -----------------------------------------------------------------------
    * Remove a tab
    * @param {number} iIndex - The index of the tab to remove
    */
   RemoveTab(iIndex) {
      if( iIndex < 0 || iIndex >= this.aTabs.length ) return;
      const oTab = this.aTabs[iIndex];

      // ## Remove the tab button and panel from the DOM .....................
      if( oTab.eButton && oTab.eButton.parentNode ) { oTab.eButton.parentNode.removeChild(oTab.eButton); }
      if( oTab.ePanel && oTab.ePanel.parentNode ) { oTab.ePanel.parentNode.removeChild(oTab.ePanel); }

      this.aTabs.splice(iIndex, 1);                                           // Remove the tab from the array

      this.aTabs.forEach((oTab, iNewIndex) => {                               // Update the tab index attributes
         oTab.eButton.setAttribute('data-tab-index', iNewIndex);
      });

      // ## If the active tab was removed, set the next tab as active .........
      if( this.iActiveIndex === iIndex ) {
         if( this.aTabs.length > 0 ) {
            const iNextActive = Math.min(iIndex, this.aTabs.length - 1);
            this.SetActiveTab(iNextActive);
         }
         else { this.iActiveIndex = -1; }
      }
      else if (this.iActiveIndex > iIndex) { this.iActiveIndex--; }
   }

   /** -----------------------------------------------------------------------
    * Getters and Helpers
    */
   GetActiveTab() { return this.iActiveIndex; }
   GetTab(iIndex) { return (iIndex >= 0 && iIndex < this.aTabs.length) ? this.aTabs[iIndex] : null; }

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

   /**
    * Retrieves property values from a source object or array.
    *
    * @param {Object|Array} source_ - The data source (mixed type).
    * @param {Number|Array<Number>|Boolean} index_ - The index, indices, or flag (mixed type).
    * @param {String|Array<String>} property_ - The property name(s) (mixed type).
    * @param {Function|Boolean} accessor_ - Optional callback or boolean (mixed type).
    * @returns {Array|Any} - Array of [index, value] or single value.
    *
    * @example
    * // Basic Usage: Get a single property from a specific index
    * const aUsers = [{ id: 1, name: "John" }, { id: 2, name: "Jane" }];
    * const sName = GetValue_s(aUsers, 0, "name");
    * console.log(sName); // Output: "John"
    *
    * @example
    * // Nested Property: Access deeply nested values using dot notation
    * const oData = { user: { details: { age: 25 } } };
    * const iAge = GetValue_s(oData, 0, "user.details.age");
    * console.log(iAge); // Output: 25
    *
    * @example
    * // Multiple Properties: Retrieve multiple keys at once (returns array of values)
    * const aProps = GetValue_s(aUsers, 1, ["id", "name"]);
    * console.log(aProps); // Output: [2, "Jane"]
    *
    * @example
    * // Multiple Indices: Get specific rows formatted as [[index, value], ...]
    * const aRows = GetValue_s(aUsers, [0, 1], "id");
    * console.log(aRows); // Output: [[0, 1], [1, 2]]
    *
    * @example
    * // Get All: Use boolean true to return the whole dataset structure
    * const aAll = GetValue_s(aUsers, true, "name");
    * console.log(aAll); // Output: [[0, "John"], [1, "Jane"]]
    *
    * @example
    * // Custom Accessor: Provide a function to calculate data dynamically
    * const fnCalc = (i) => ({ val: i * 10 });
    * const aCalc = GetValue_s(aUsers, [1, 2], "val", fnCalc);
    * console.log(aCalc); // Output: [[1, 10], [2, 20]]
    *
    */
   static GetValue_s(source_, index_, property_, accessor_) {
       // ## 1. Normalize Source: Ensure we are working with an array .........
       let aSource = Array.isArray(source_) ? source_ : [source_];

       // ## 2. Normalize Indices & Determine Return Format ...................
       let aiTarget = [];        // Array of Integers: target indices
       let bReturnArray = false; // Boolean: return format flag

       if(index_ === true || index_ === void 0) {                             // "Get All" mode
           aiTarget = aSource.map((_, i) => i);
           bReturnArray = true;
       }
       else if(Array.isArray(index_)) {                                      // Specific list of indices
           aiTarget = index_;
           bReturnArray = index_.length > 1;
       }
       else {                                                                 // Single index passed as a number/string
           aiTarget = [index_];
           bReturnArray = false;
       }

       // Helper: Safe deep property extraction
       // o = Object, s = String
       const get_value_ = (oObj, sPath) => {
           return sPath.split('.').reduce((oAcc, sPart) => (oAcc && oAcc[sPart] !== undefined) ? oAcc[sPart] : undefined, oObj);
       };

       let aResult = []; // Array: collection of results

       // ## 3. Iterate and Extract ...........................................
       for (let i = 0; i < aiTarget.length; i++) {
           let iIdx = aiTarget[i]; // Integer: current index
           let oItem;             // Object: current item

           // Retrieve object using accessor (function) or default array access
          if(typeof accessor_ === 'function') { oItem = accessor_(iIdx); }
          else { oItem = aSource[iIdx]; }

           let value_; // Mixed: extracted value (primitive or array)

           if(Array.isArray(property_)) {
               // Map over properties (Array of Strings -> Array of Values)
               value_ = property_.map(sProp => get_value_(oItem, sProp));
           }
           else {  value_ = get_value_(oItem, property_); }                   // Single property (String -> Value)

           aResult.push([iIdx, value_]);
       }

       // ## 4. Format Return Value ...........................................
       if(index_ === true) return aResult;
       if(bReturnArray) return aResult;

       // Single item requested: return just the value
       return (aResult.length === 1) ? aResult[0][1] : null;
   }
}
