/**
 * Form - A class for managing form fields with metadata for single record editing.
 *
 * **Quick Start:**
 * ```javascript
 * // Create form with field definitions
 * const form = new Form([
 *   { sId: "name", sLabel: "Full Name", sType: "string", bRequired: true, iMinLength: 2 },
 *   { sId: "age", sLabel: "Age", sType: "number", nMin: 18, nMax: 100 },
 *   { sId: "email", sLabel: "Email", sType: "string", pattern: /^[^\s@]+@[^\s@]+\.[^\s@]+$/ },
 *   { sId: "status", sLabel: "Status", sType: "string", aAllowedValues: ["active", "inactive", "pending"] }
 * ]);
 *
 * // Set field values
 * form.SetValue("name", "John Doe");
 * form.SetValue("age", 30);
 * form.SetValue("status", "active");
 *
 * // Get all values as object
 * const data = form.GetValues();
 *
 * // Validate all fields
 * const isValid = form.Validate();
 * ```
 *
 * **Key Concepts:**
 * - **Fields:** Define structure (id, label, type, validation, description). Access via id.
 * - **Values:** Stored separately from field metadata.
 * - **Validation:** Each field can have validation rules and error messages.
 * - **UI Metadata:** Fields store information needed for rendering (label, description, state).
 *
 * **Common Methods:**
 * - `GetValue(id)` - Get value for a field
 * - `SetValue(id, value)` - Set value for a field
 * - `GetValues()` - Get all values as object
 * - `Validate()` - Validate all fields
 * - `GetField(id)` - Get field metadata
 */
class Form {

   static field = class {
      /**
       * @param {Object|string} options_ - Either a configuration object or the field id string
       * @param {string} options_.sId - Unique identifier for the field
       * @param {string} [options_.sLabel] - Display label for the field
       * @param {string} [options_.sType="string"] - Field type (string, number, boolean, date)
       * @param {string} [options_.sDescription=""] - Help text or description
       * @param {boolean} [options_.bRequired=false] - Whether field is required
       * @param {boolean} [options_.bReadonly=false] - Whether field is readonly
       * @param {boolean} [options_.bDisabled=false] - Whether field is disabled
       * @param {number} [options_.iState=0] - Bitwise state flags (1: error, 2: modified, 4: focused)
       * @param {Function|null} [options_.fnValidate=null] - Custom validation function
       * @param {string|RegExp|null} [options_.pattern=null] - Regex pattern for validation
       * @param {number|null} [options_.iMinLength=null] - Minimum string length
       * @param {number|null} [options_.iMaxLength=null] - Maximum string length
       * @param {number|null} [options_.nMin=null] - Minimum number value
       * @param {number|null} [options_.nMax=null] - Maximum number value
       * @param {Array|null} [options_.aAllowedValues=null] - Array of allowed values
       * @param {string} [options_.sError=""] - Current error message
       * @param {any} [options_.default_value_=null] - Default value for the field
       * @param {Object} [options_.oMetadata={}] - Additional metadata for UI rendering
       */
      constructor(options_ = {}) {
         if( typeof options_ === "string" ) { options_ = {sId: options_}; }
         if( typeof options_ !== "object" ) { throw new Error("Invalid argument"); }

         const oOptions = Object.assign({
            sId: "",              sLabel: "",           sType: "string",
            sDescription: "",     bRequired: false,     bReadonly: false,
            bDisabled: false,     iState: 0,            fnValidate: null,
            pattern: null,        iMinLength: null,     iMaxLength: null,
            nMin: null,           nMax: null,           aAllowedValues: null,
            sError: "",           default_value_: null, oMetadata: {}
         }, options_);

         this.sId = oOptions.sId || "";
         this.sLabel = oOptions.sLabel || this.sId;
         this.sType = oOptions.sType || "string";
         this.sDescription = oOptions.sDescription || "";
         this.bRequired = oOptions.bRequired || false;
         this.bReadonly = oOptions.bReadonly || false;
         this.bDisabled = oOptions.bDisabled || false;
         this.iState = oOptions.iState || 0;
         this.fnValidate = oOptions.fnValidate || null;
         this.pattern = oOptions.pattern || null;
         this.iMinLength = oOptions.iMinLength !== null ? oOptions.iMinLength : null;
         this.iMaxLength = oOptions.iMaxLength !== null ? oOptions.iMaxLength : null;
         this.nMin = oOptions.nMin !== null ? oOptions.nMin : null;
         this.nMax = oOptions.nMax !== null ? oOptions.nMax : null;
         this.aAllowedValues = oOptions.aAllowedValues || null;
         this.sError = oOptions.sError || "";
         this.default_value_ = oOptions.default_value_ !== undefined ? oOptions.default_value_ : null;
         this.oMetadata = oOptions.oMetadata || {};

         if( this.pattern && typeof this.pattern === "string" ) { this.pattern = new RegExp(this.pattern); } // Convert string pattern to RegExp if needed
      }

      // Type checks
      is_string() { return this.sType === "string"; }
      is_number() { return this.sType === "number"; }
      is_boolean() { return this.sType === "boolean"; }
      is_date() { return this.sType === "date"; }

      // State checks
      has_error() { return (this.iState & 1) === 1; }
      is_modified() { return (this.iState & 2) === 2; }
      is_focused() { return (this.iState & 4) === 4; }

      // State setters
      set_error(bError) {
         if( bError ) { this.iState |= 1; }
         else { this.iState &= ~1; }
      }

      set_modified(bModified) {
         if( bModified ) { this.iState |= 2; }
         else { this.iState &= ~2; }
      }

      set_focused(bFocused) {
         if( bFocused ) { this.iState |= 4; }
         else { this.iState &= ~4; }
      }
   }


   // @API [tag: form, construction] [summary: create form]

   /** -----------------------------------------------------------------------
    * Constructor
    *
    * @param {Array<Object>} fields_ - Array of field definitions
    * @param {Object} [options_={}] - Additional options
    * @param {Object} [options_.oValues={}] - Initial values for fields
    */
   constructor(fields_ = [], options_ = {}) {
      let aField;
      if( !Array.isArray(fields_) ) { throw new Error("Invalid argument"); }

      aField = fields_;

      const oOptions = Object.assign({ oValues: {} }, options_);

      // ## Initialize fields with metadata
      this.aField = aFields_.map(field_ => new Form.field(field_));

      // ## Initialize values object
      this.oValue = {};
      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];
         this.oValue[oField.sId] = oOptions.oValues[oField.sId] !== undefined
            ? oOptions.oValues[oField.sId]
            : oField.default_value_;
      }

      // ## Track original values for dirty checking
      this.oOriginalValue = { ...this.oValue };
   }

   /** -----------------------------------------------------------------------
    * Get field index by id
    *
    * @param {string} sId_ - Field id to search for
    * @returns {number} index to field or -1 if not found
    */
   GetFieldIndex(sId_) {
      for( let i = 0; i < this.aField.length; i++ ) {
         if( this.aField[i].sId === sId_ ) { return i; }
      }
      return -1;
   }

   /** -----------------------------------------------------------------------
    * Get field metadata by id or index
    *
    * @param {string|number} field_ - Field id or index
    * @returns {Form.field|null} Field object or null if not found
    */
   GetField(field_) {
      let iIndex = field_;
      if( typeof field_ === "string" ) { iIndex = this.GetFieldIndex(field_); }
      if( iIndex < 0 || iIndex >= this.aField.length ) { return null; }

      return this.aField[iIndex];
   }

   /** -----------------------------------------------------------------------
    * Get value for a field
    *
    * @param {string} sId_ - Field id
    * @returns {any} Field value or null if field not found
    */
   GetValue(sId_) {
      if( this.oValue.hasOwnProperty(sId_) ) {
         return this.oValue[sId_];
      }
      return null;
   }

   /** -----------------------------------------------------------------------
    * Set value for a field
    *
    * @param {string} sId_ - Field id
    * @param {any} value_ - Value to set
    * @param {boolean} [bMarkModified=true] - Whether to mark field as modified
    * @returns {boolean} True if value was set, false if field not found
    */
   SetValue(sId_, value_, bMarkModified = true) {
      const oField = this.GetField(sId_);
      if( !oField ) { return false; }

      this.oValue[sId_] = value_;

      if( bMarkModified ) {
         // ## Check if value differs from original
         if( this.oOriginalValue[sId_] !== value_ ) {
            oField.set_modified(true);
         }
         else {
            oField.set_modified(false);
         }
      }

      return true;
   }

   /** -----------------------------------------------------------------------
    * Get all values as an object
    *
    * @param {Object} [options_={}] - Options for getting values
    * @param {boolean} [options_.bModifiedOnly=false] - Only return modified values
    * @returns {Object} Object with field ids as keys and values
    */
   GetValues(options_ = {}) {
      const oOptions = Object.assign({ bModifiedOnly: false }, options_);

      if( !oOptions.bModifiedOnly ) {
         return { ...this.oValue };
      }

      // ## Return only modified values
      const oModified = {};
      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];
         if( oField.is_modified() ) {
            oModified[oField.sId] = this.oValue[oField.sId];
         }
      }

      return oModified;
   }

   /** -----------------------------------------------------------------------
    * Set multiple values at once
    *
    * @param {Object} oValues_ - Object with field ids as keys
    * @param {boolean} [bMarkModified=true] - Whether to mark fields as modified
    */
   SetValues(oValues_, bMarkModified = true) {
      for( const [sId_, value_] of Object.entries(oValues_) ) {
         this.SetValue(sId_, value_, bMarkModified);
      }
   }

   /** -----------------------------------------------------------------------
    * Validate a single field
    *
    * @param {string} sId_ - Field id
    * @returns {boolean} True if field is valid
    */
   ValidateField(sId_) {
      const oField = this.GetField(sId_);
      if( !oField ) { return false; }

      const value_ = this.oValue[sId_];

      // ## Clear previous error
      oField.sError = "";
      oField.set_error(false);

      // ## Check required ....................................................
      if( oField.bRequired ) {
         if( value_ === null || value_ === undefined || value_ === "" ) {
            oField.sError = `${oField.sLabel} is required`;
            oField.set_error(true);
            return false;
         }
      }

      // ## Skip validation for empty non-required fields
      if( value_ === null || value_ === undefined || value_ === "" ) {
         return true;
      }

      // ## Type validation ...................................................
      if( oField.is_number() ) {
         const nValue = Number(value_);
         if( isNaN(nValue) ) {
            oField.sError = `${oField.sLabel} must be a number`;
            oField.set_error(true);
            return false;
         }

         // ## Min/Max validation for numbers
         if( oField.nMin !== null && nValue < oField.nMin ) {
            oField.sError = `${oField.sLabel} must be at least ${oField.nMin}`;
            oField.set_error(true);
            return false;
         }

         if( oField.nMax !== null && nValue > oField.nMax ) {
            oField.sError = `${oField.sLabel} must be at most ${oField.nMax}`;
            oField.set_error(true);
            return false;
         }
      }

      // ## String length validation .........................................
      if( oField.is_string() ) {
         const sValue = String(value_);

         if( oField.iMinLength !== null && sValue.length < oField.iMinLength ) {
            oField.sError = `${oField.sLabel} must be at least ${oField.iMinLength} characters`;
            oField.set_error(true);
            return false;
         }

         if( oField.iMaxLength !== null && sValue.length > oField.iMaxLength ) {
            oField.sError = `${oField.sLabel} must be at most ${oField.iMaxLength} characters`;
            oField.set_error(true);
            return false;
         }

         // ### Pattern validation (regex)
         if( oField.pattern ) {
            if( !oField.pattern.test(sValue) ) {
               oField.sError = `${oField.sLabel} format is invalid`;
               oField.set_error(true);
               return false;
            }
         }
      }

      // ## Allowed values validation .........................................
      if( oField.aAllowedValues && Array.isArray(oField.aAllowedValues) ) {
         if( !oField.aAllowedValues.includes(value_) ) {
            oField.sError = `${oField.sLabel} must be one of: ${oField.aAllowedValues.join(", ")}`;
            oField.set_error(true);
            return false;
         }
      }

      // ## Custom validation ................................................
      if( oField.fnValidate ) {
         const result_ = oField.fnValidate(value_, this);
         if( result_ !== true ) {
            oField.sError = typeof result_ === "string" ? result_ : `${oField.sLabel} is invalid`;
            oField.set_error(true);
            return false;
         }
      }

      return true;
   }

   /** -----------------------------------------------------------------------
    * Validate all fields
    *
    * @returns {boolean} True if all fields are valid
    */
   Validate() {
      let bValid = true;

      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];
         if( !this.ValidateField(oField.sId) ) {
            bValid = false;
         }
      }

      return bValid;
   }

   /** -----------------------------------------------------------------------
    * Get validation errors for all fields
    *
    * @returns {Object} Object with field ids as keys and error messages as values
    */
   GetErrors() {
      const oErrors = {};

      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];
         if( oField.has_error() && oField.sError ) {
            oErrors[oField.sId] = oField.sError;
         }
      }

      return oErrors;
   }

   /** -----------------------------------------------------------------------
    * Check if form has any errors
    *
    * @returns {boolean} True if form has errors
    */
   HasErrors() {
      for( let i = 0; i < this.aField.length; i++ ) {
         if( this.aField[i].has_error() ) { return true; }
      }
      return false;
   }

   /** -----------------------------------------------------------------------
    * Check if form has been modified
    *
    * @returns {boolean} True if any field has been modified
    */
   IsDirty() {
      for( let i = 0; i < this.aField.length; i++ ) {
         if( this.aField[i].is_modified() ) { return true; }
      }
      return false;
   }

   /** -----------------------------------------------------------------------
    * Reset form to original values
    */
   Reset() {
      this.oValue = { ...this.oOriginalValue };

      // ## Clear all modified flags and errors
      for( let i = 0; i < this.aField.length; i++ ) {
         this.aField[i].set_modified(false);
         this.aField[i].set_error(false);
         this.aField[i].sError = "";
      }
   }

   /** -----------------------------------------------------------------------
    * Accept current values as the new baseline (clear modified flags)
    */
   AcceptChanges() {
      this.oOriginalValue = { ...this.oValue };

      for( let i = 0; i < this.aField.length; i++ ) {
         this.aField[i].set_modified(false);
      }
   }

   /** -----------------------------------------------------------------------
    * Get count of fields
    *
    * @returns {number} Number of fields in form
    */
   GetFieldCount() { return this.aField.length; }

   /** -----------------------------------------------------------------------
    * Add a new field to the form
    *
    * @param {Object} oField_ - Field definition
    * @param {any} [value_=null] - Initial value for the field
    */
   AddField(oField_, value_ = null) {
      const oNewField = new Form.field(oField_);
      this.aField.push(oNewField);

      const initialValue_ = value_ !== null ? value_ : oNewField.default_value_;
      this.oValue[oNewField.sId] = initialValue_;
      this.oOriginalValue[oNewField.sId] = initialValue_;
   }

   /** -----------------------------------------------------------------------
    * Remove a field from the form
    *
    * @param {string} sId_ - Field id to remove
    * @returns {boolean} True if field was removed
    */
   RemoveField(sId_) {
      const iIndex = this.GetFieldIndex(sId_);
      if( iIndex === -1 ) { return false; }

      this.aField.splice(iIndex, 1);
      delete this.oValue[sId_];
      delete this.oOriginalValue[sId_];

      return true;
   }

   // @API [tag: form, utilities] [description: Helper methods for form manipulation]

   /** -----------------------------------------------------------------------
    * Get form data suitable for submission (e.g., to an API)
    *
    * @param {Object} [options_={}] - Options for data format
    * @param {boolean} [options_.bModifiedOnly=false] - Only include modified fields
    * @param {boolean} [options_.bExcludeReadonly=false] - Exclude readonly fields
    * @returns {Object} Form data object
    */
   GetFormData(options_ = {}) {
      const oOptions = Object.assign({
         bModifiedOnly: false,
         bExcludeReadonly: false
      }, options_);

      const oData = {};

      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];

         // ## Skip based on options
         if( oOptions.bModifiedOnly && !oField.is_modified() ) { continue; }
         if( oOptions.bExcludeReadonly && oField.bReadonly ) { continue; }

         oData[oField.sId] = this.oValue[oField.sId];
      }

      return oData;
   }

   /** -----------------------------------------------------------------------
    * Clear all values in the form
    */
   Clear() {
      for( let i = 0; i < this.aField.length; i++ ) {
         const oField = this.aField[i];
         this.oValue[oField.sId] = oField.default_value_;
      }
   }
}
