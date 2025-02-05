
/**
 * Toaster class to show messages to the user. The toaster is a simple component that can be used to show
 * messages to the user. it has functionality to style it and also to show different types of messages.
 * You can add a callback to pickup the message and do something with it.
 */
export default class CToaster {
   static m_sWidgetName_s = 'uitoaster';
   /**
    * Creates an instance of the CToaster.
    *
    * @constructor
    * @param {Object} [options={}] - The configuration options for the toaster.
    * @param {number} [options.duration=3000] - The duration (in milliseconds) for which the toaster message is displayed.
    * @param {string} [options.type='info'] - The type of the toaster message (e.g., 'primary', 'secondary', 'success', 'danger', 'warning', 'info').
    * @param {Function|Function[]} [options.callback] - A callback function or an array of callback functions for toaster operations.
    * @param {HTMLElement} [options.parent=document.body] - The parent DOM element to which the toaster component is appended.
    * @param {Object} [options.style] - CSS styles for the toaster's container element.
    * @param {Object} [options.toaster_style] - CSS styles for the individual toaster message element.
    *
    * @example
    * const toaster = new CToaster({
    *    duration: 5000,
    *    type: 'success',
    *    callback: () => console.log('Toaster action executed!'),
    *    parent: document.getElementById('toaster-container'),
    *    style: { "margin-bottom": "20px", "padding": "10px" },
    *    toaster_style: { "border-radius": "5px", "color": "#fff" }
    * });
    */
   constructor(options) {
      // callback function to call for operations in toaster
      this.m_acallback = [];
      if (options.callback) this.m_acallback = Array.isArray(options.callback) ? options.callback : [options.callback];

      this.#call("before-construct", options );

      const o = options || {};

      // ## data memebers
      this.m_iDuration = o.duration || 3000; // duration of the toaster message
      this.m_sType = o.type || 'info'; // type of the toaster message

      // unique id for the component
      this.m_sId = CToaster.m_sWidgetName_s + `${Date.now()}-${Math.floor(Math.random() * 1000000)}`;

      // ## elements and related element data
      this.m_eComponent = null; // The component element, this core element is the parent of all other elements
      this.m_eParent = o.parent || document.body; // owner of the component

      this.m_oStyle = o.style || { "margin-bottom": "20px", "padding": "10px 20px", "position": "fixed", "right": "20px", "z-index": 1000 };
      this.m_oToasterStyle = o.toaster_style || { "border-radius": '5px', "margin-bottom": '10px', "opacity": '0', "padding": '10px 20px', "transition": 'opacity 0.5s ease-in-out' };
   }

   get id() { return this.m_sId; }
   get parent() { return this.m_eParent; }
   get component() { return this.m_eComponent; }

   /**
    * Get the componenet element for the toaster
    * @param {boolean} bCreate if true then create component if not found
    * @returns {HTMLElement} to toaster component
    */
   GetComponent(bCreate = false) 
   {
      if( this.m_eComponent ) return this.m_eComponent;
      if( bCreate === true ) this.Create();

      return this.m_eComponent
   }

   /**
    * Create the toaster component. The toaster need to be attached to a parent where the internal
    * `m_eComponent` is added to.
    * @example
    * const toaster = new CToaster({ duration: 5000 });
    * toaster.Create(".toaster", "width: 500px;");
    * toaster.Show('Success! It worked.', 'success');
    * @param {HTMLElement|string} [parent_] parent element, id or selector of the parent element
    * @param {object|string} [style_] style for the component, can both css text or object
    */
   Create( parent_, style_ ) 
   {
      let eParent; // HTML element to hold the component
      if(typeof parent_ === "string") 
      {
         eParent = document.getElementById(parent_);
         if(eParent === null) { eParent = document.querySelector(parent_); }
      }
      else { eParent = parent_ || this.m_eParent; }

      let eComponent = document.createElement('div');
      Object.assign(eComponent.dataset, { section: "component", id: this.m_sId, widget: CToaster.m_sWidgetName_s }); // set data- items
      Object.assign(eComponent.style, this.m_oStyle); // set style

      if(typeof style_ === "string") { let s_ = eComponent.style.cssText || ""; eComponent.style.cssText = s_ + style_; }
      else if(typeof style_ === "object") Object.assign(eComponent.style, style_);

      eParent.appendChild(eComponent);
      this.m_eComponent = eComponent;
      console.log(eComponent.cssText);
   }

   /**
    * Destroys the toaster component by removing it from the DOM.
    *
    * If the component element exists, this method removes it from the DOM and then nullifies
    * the reference to ensure proper cleanup.
    *
    * @returns {void}
    */
   Destroy() {
      if(this.m_eComponent === null) return;
      this.#call("destroy");
      this.m_eComponent.remove();
      this.m_eComponent = null;
   }


   /**
    * Show a toaster message. There are six core types: primary, secondary, success, danger, warning, info
    * for toaster to work with and default is to set the background color to the variable in the css
    * named background-{type}.
    * @param {string} sMessage message to show for user
    * @param {string} sType type of message, can be primary, secondary, success, danger, warning, info
    * @param {number} iDuration how long the message should be shown in milliseconds
    */
   Show(sMessage, sType, iDuration) 
   {                                                                                               
      // Get the main component element that acts as container for toaster messages
      let eComponent = this.GetComponent( true );                                                  console.assert( sMessage !== undefined, "Message is required"); console.assert(this.m_eComponent !== null, "Component not created");

      sType = sType || this.m_sType; // type of message
      iDuration = iDuration || this.m_iDuration;// duration of the message

      const eToaster = document.createElement('div');
      Object.assign(eToaster.style, this.m_oToasterStyle); // set style for toaster message 
      eToaster.textContent = sMessage;
      let sBackgroundColor = sType;

      /// ## Compare against core color types
      if(["primary", "secondary", "success", "danger", "warning", "info"].includes(sType) === true) 
      {
         sBackgroundColor = `var(--background-${sType})`; // get the color from the css variable
      }
      
      eToaster.style.backgroundColor = sBackgroundColor;                      // set the background style


      eComponent.appendChild(eToaster);

      // // show the toaster
      setTimeout(() => { 
         eToaster.style.opacity = '1'; this.#call("show", {message: sMessage} ); 
      }, 10); 

      // callback method to remove the toaster
      const remove_element_ = () => { 
         eComponent.removeChild(eToaster); 
         this.#call("close", { message: sMessage });
      }
      // ## Remove the toaster after the duration has elapsed, this is done because await fading out the toaster
      setTimeout( () => { eToaster.style.opacity = '0'; 
         setTimeout(() => { remove_element_(); }, 500);
      }, iDuration);
   }

   /**
    * Hides the toaster component.
    *
    * This method retrieves the component element by calling {@link CToaster#GetComponent}.
    * If the component exists, its display style is set to "none", effectively hiding it.
    * Afterwards, it triggers any registered "hide" callbacks via a private method.
    *
    * @returns {void}
    */
   Hide() {
      let eComponent = this.GetComponent();
      if(eComponent === null) return;

      eComponent.style.display = "none";
      this.#call("hide");
   }

   /**
    * private method to call methods in connected methods found in m_acallback
    * @param {string} sMessage name about the event
    * @param {any} [data_] any data to pass to the callback
    * @param {object} [oEVT] event object when browser event is passed
    */
   #call(sMessage, data_, oEVT ) 
   {
      for (let i = 0; i < this.m_acallback.length; i++) 
      {
         const call_ = this.m_acallback[i];
         call_.call( this, sMessage, data_, oEVT );
      }
   }
}
