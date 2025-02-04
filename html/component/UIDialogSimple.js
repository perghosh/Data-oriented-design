/**
 * CModalSimple class to display a modal dialog to the user.
 * This modal is a simple component that can be used to show a title,
 * content, and includes a close button. It also supports callbacks for events.
 */
export default class CModalSimple {
   static m_sWidgetName_s = 'uimodalsimple';
   
   /**
    * Constructor for the modal dialog.
    * @param {object} options - Options for the modal dialog.
    *   @property {string} [type] - Type of dialog.
    *   @property {string} [title] - The modal title.
    *   @property {string|HTMLElement} [content] - The modal content (can be HTML string or an element).
    *   @property {HTMLElement|string} [parent] - Parent element or selector for attaching the modal.
    *   @property {object} [style] - CSS styles for the overlay.
    *   @property {object} [style_content] - CSS styles for the modal content.
    *   @property {object} [style_close] - CSS styles for the close button.
    *   @property {function|function[]} [callback] - Callback function(s) to invoke on modal events.
    */
   constructor(options) {
      const o = options || {};
      
      // Data members
      this.m_sType = o.type || 'secondary';
      this.m_sTitle = o.title || 'Modal Title';
      this.m_content_ = o.content || 'This is a modal dialog.';
      this.m_acallback = [];
      if(o.callback) this.m_acallback = Array.isArray(o.callback) ? o.callback : [o.callback];

      // Unique id for the component
      this.m_sId = CModalSimple.m_sWidgetName_s + `${Date.now()}-${Math.floor(Math.random() * 1000000)}`;

      // Elements and related element data
      this.m_eComponent = null; // The root element for the modal
      this.m_eParent = o.parent || document.body; // Parent element for the modal

      // Default styles for the overlay
      this.m_oStyle = o.style || {"alignItems": "center", "backgroundColor": "rgba(0, 0, 0, 0.5)", "display": "flex", "height": "100%", "left": "0", "opacity": "0", "position": "fixed", "justifyContent": "center", "top": "0", "transition": "opacity 0.3s ease-in-out", "width": "100%", "zIndex": "1000"};
      // Default styles for the modal content
      this.m_oStyleContent = o.style_content || {"backgroundColor": "#fff", "borderRadius": "5px", "minHeight": "150px", "minWidth": "300px", "padding": "20px", "position": "relative"};
      // Default styles for the close button
      this.m_oStyleClose = o.style_close || {"cursor": "pointer", "fontSize": "18px", "fontWeight": "bold", "position": "absolute", "right": "10px", "top": "10px"};
   }

   get id() { return this.m_sId; }
   get parent() { return this.m_eParent; }
   get component() { return this.m_eComponent; }

   /**
    * Get the modal component. If it doesn't exist and bCreate is true, then it is created.
    * @param {boolean} bCreate - Whether to create the modal if not found.
    * @returns {HTMLElement} The modal element.
    */
   GetComponent(bCreate = false) {
      if( this.m_eComponent ) return this.m_eComponent;
      if( bCreate === true ) this.Create();

      return this.m_eComponent
   }

   /**
    * Create the modal component and attach it to the parent.
    * @param {HTMLElement|string} [parent_] - Parent element, id, or selector.
    * @param {object|string} [style_] - Additional styles for the overlay.
    */
   Create(parent_, style_) {
      let eParent;
      if(typeof parent_ === "string") 
      {
         eParent = document.getElementById(parent_);
         if(eParent === null) { eParent = document.querySelector(parent_); }
      } 
      else {  eParent = parent_ || this.m_eParent; }

      // Create overlay element for the modal
      const eComponent = document.createElement('div');
      Object.assign(eComponent.dataset, { section: "component", id: this.m_sId, widget: CModalSimple.m_sWidgetName_s });
      Object.assign(eComponent.style, this.m_oStyle);
      if(typeof style_ === "string") { let s_ = eComponent.style.cssText || ""; eComponent.style.cssText = s_ + style_; } 
      else if(typeof style_ === "object") { Object.assign(eComponent.style, style_); }

      // Create modal content container
      const eContent = document.createElement('div');
      eContent.className = "modal-content";
      Object.assign(eContent.style, this.m_oStyleContent);

      let sColor = "";
      let sBackgroundColor = this.m_sType;
      /// ## Compare against core color types
      const sType = this.m_sType.toLowerCase();
      if(["primary", "secondary", "success", "danger", "warning", "info"].includes(sType) === true) 
      {
         sBackgroundColor = `var(--background-${sType})`; // get the color from the css variable
         sColor = `var(--color-${sType})`; // get the color from the css variable
      }
      
      eContent.style.backgroundColor = sBackgroundColor;                     // set the background style
      if(sColor) eContent.style.color = sColor;                              // set the color style


      // Create and append the title element if provided
      if( this.m_sTitle )  
      {
         const eTitle = document.createElement('h2');
         eTitle.textContent = this.m_sTitle;
         eContent.appendChild(eTitle);
      }

      // Create and append the body element with modal content
      const eBody = document.createElement('div');
      eBody.className = "modal-body";
      if(typeof this.m_content_ === "string") 
      {
         eBody.innerHTML = this.m_content_;
      } 
      else if(this.m_content_ instanceof HTMLElement) 
      {
         eBody.appendChild(this.m_content_);
      }
      eContent.appendChild(eBody);

      // ## Create and append the close button
      const eClose = document.createElement('span');
      eClose.textContent = '×';
      Object.assign(eClose.style, this.m_oStyleClose);
      // ### Bind the close action on click
      eClose.addEventListener("click", () => this.Hide());
      eContent.appendChild(eClose);

      // ## Append content to the overlay element
      eComponent.appendChild(eContent);

      // ## Close modal when clicking on the overlay but outside the content area
      eComponent.addEventListener("click", (event) => {
         if(event.target === eComponent) { this.Hide(); }
      });

      eParent.appendChild(eComponent);
      this.m_eComponent = eComponent;
      return this.m_eComponent;
   }

   /**
    * Show the modal dialog.
    */
   Show( content_, sType ) 
   {
      const eComponent = this.GetComponent(true);

      // Allow the CSS transition to kick in
      setTimeout(() => {
         eComponent.style.opacity = "1";
         this.#call("show");
      }, 10);
   }

   /**
    * Hide the modal dialog. 
    */
   Hide() 
   {
      if(!this.m_eComponent) return;
      this.m_eComponent.style.opacity = "0";
      // set style to none
      this.m_eComponent.style.display = "none;
      this.#call("hide");
   }

   /**
    * Destroy the modal dialog immediately.
    */
   Destroy() {
      if(!this.m_eComponent) return;
      this.m_eComponent.remove();
      this.m_eComponent = null;
   }

   /**
    * Private method to invoke registered callbacks.
    * @param {string} sMessage - The event name.
    * @param {any} [data_] - Additional data to pass to the callback.
    * @param {object} [oEVT] - The event object.
    */
   #call(sMessage, data_, oEVT) {
      for(let i = 0; i < this.m_acallback.length; i++) {
         const call_ = this.m_acallback[i];
         call_.call(this, sMessage, data_, oEVT);
      }
   }
}
