
export class column {
  // Define default values as static properties
   static DEFAULT_TYPE = "string";
   static DEFAULT_ALIGN = "left";
   static DEFAULT_SORTED = "";
   static DEFAULT_WIDTH = -1;
   static DEFAULT_HIDE = false;

   static ARRAY_TYPE = ["string", "integer", "decimal", "date", "binary", "boolean"];
   static ARRAY_ALIGN = ["left", "center", "right"];
   static ARRAY_SORT = ["asc", "desc", ""];

   // ## Static methods

   /**
    * Return the default alignment for a given type.
    * @param {any} sType - The type of the column (e.g., "integer", "decimal", "string", "date", "binary", "boolean").
    * @returns {string} The default alignment for the given type.
    */
   static align_for_type_s(sType) {
      if(!sType) sType = column.DEFAULT_TYPE; // If no type is provided, use the default type
      if(sType === "string") return "left"; // Default alignment for string type
      return "right"; // Default alignment for other types is right
   }


   /**
    * Constructs a Column object to represent a column in a table.
    * @param {string|Object} options_ - Either a string representing the column name or an object containing column properties.
    * @param {string} [sType] - The data type of the column (e.g., "integer", "decimal", "string", "date", "binary", "boolean"). Defaults to column.DEFAULT_TYPE.
    * @param {string} [sAlias] - The alias for the column
    * @param {string} [sAlign] - The alignment of the column (e.g., "left", "center", "right"). Defaults to column.DEFAULT_ALIGN.
    * @constructor
    * @example
    * // String-based constructor
    * new Column("ID", "integer", "center");
    * // Object-based constructor
    * new Column({ name: "ID", type: "integer", align: "center", alias: "Identifier" });
    */
   constructor(options_, sType, sAlias, sAlign) {                                                          console.assert( typeof options_ === 'string' || typeof options_ === 'object', "Invalid type for options_: expected string or object" );
      let sName;
      if(typeof options_ == 'string') {
         sName = options_; // e.g., "ID", "Name"
         options_ = {};
         this.m_sName = sName; // e.g., "ID", "Name"
         this.m_sAlias = sAlias ?? sName; // e.g., alias for name or name if no alias
         this.m_sType = sType ?? column.DEFAULT_TYPE; // e.g., "integer", "decimal", "string", "date", "binary", "boolean"
         this.m_sAlign = sAlign ?? column.align_for_type_s(this.m_sType); // e.g., "left", "center", "right"
         this.m_sSorted = column.DEFAULT_SORTED; // e.g., "asc", "desc", ""
         this.m_iWidth = column.DEFAULT_WIDTH; // e.g., 10, 100 or -1 no width
         this.m_bHide = column.DEFAULT_HIDE; // e.g., true, false
      }
      else {
         // Handle case where all values are passed in options_ object
         this.m_sName = options_.name || ""; // Set name from options or empty string if not provided
         this.m_sAlias = options_.alias ?? this.m_sName; // Set alias from options or name if not provided
         this.m_sType = options_.type ?? column.DEFAULT_TYPE; // Set type from options or default to "string"
         this.m_sAlign = options_.align ?? column.align_for_type_s(this.m_sType); // Set alignment from options or default to type
         this.m_sSorted = options_.sorted ?? column.DEFAULT_SORTED; // Set sorted from options or default to ""
         this.m_iWidth = options_.width !== undefined ? options_.width : column.DEFAULT_WIDTH; // Set width from options or default to -1
         this.m_bHide = options_.hide ?? column.DEFAULT_HIDE; // Set hide from options or default to false      }
      }
   }

   // ## Operations

   /**
    * returns true if the column is sorted
    * @returns {boolean} true if the column is sorted
    */
   is_sorted() { return this.m_sSorted !== ""; }

   set_align_for_type(sType) {
      if(!sType) sType = this.m_sType; // If no type is provided, use the current type
                                                                                                   console.assert(column.ARRAY_TYPE.includes(sType), `Invalid type: ${sType} is not in ${column.ARRAY_TYPE}`);
      if(sType === "string") { this.m_sAlign = "left"; }                       // Default alignment for string type
      else { this.m_sAlign }                                                   // Default alignment for other types
   }

   // ## Getters and Setters
   get name() { return this.m_sName; }
   set name(value) { this.m_sName = value; }
  
   get alias() { return this.m_sAlias; }
   set alias(sAlias) { this.m_sAlias = sAlias; }
  
   get type() { return this.m_sType; }
   set type(sType) { if(column.ARRAY_TYPE.includes(sType)) this.m_sType = sType; }
  
   get align() { return this.m_sAlign; }
   set align(sAlign) { if(column.ARRAY_ALIGN.includes(sAlign)) this.m_sAlign = sAlign; }
  
   get sorted() { return this.m_sSorted; }
   set sorted(sSorted) { if(column.ARRAY_SORT.includes(sSorted))this.m_sSorted = sSorted; }
  
   get width() { return this.m_iWidth; }
   set width(value) { this.m_iWidth = value; }
  
   get hide() { return this.m_bHide; }
   set hide(value) { this.m_bHide = value; }
  
   get_display_name() { return this.m_sAlias || this.m_sName; }

   // ## Utility methods

   clone() { 
      return new column({ name: this.m_sName, alias: this.m_sAlias, type: this.m_sType, align: this.m_sAlign, sorted: this.m_sSorted, width: this.m_iWidth, hide: this.m_bHide });
   }

   // ## Debugging

   // Assert method to validate internal state
   assert_valid() {
      console.assert(typeof this.m_sName === 'string' && this.m_sName.length > 0, "Invalid column name: must be a non-empty string");
      console.assert(typeof this.m_sAlias === 'string', "Invalid column alias: must be a string");
      console.assert(column.ARRAY_TYPE.includes(this.m_sType), `Invalid column type: ${this.m_sType} is not in ${column.ARRAY_TYPE}`);
      console.assert(column.ARRAY_ALIGN.includes(this.m_sAlign), `Invalid column alignment: ${this.m_sAlign} is not in ${column.ARRAY_ALIGN}`);
      console.assert(column.ARRAY_SORT.includes(this.m_sSorted), `Invalid column sort order: ${this.m_sSorted} is not in ${column.ARRAY_SORT}`);
      console.assert(typeof this.m_iWidth === 'number' && (this.m_iWidth > 0 || this.m_iWidth === -1), "Invalid column width: must be a positive number or -1");
      console.assert(typeof this.m_bHide === 'boolean', "Invalid column hide flag: must be a boolean");
   }
}

export class columns {
   constructor(aColumns = []) {
      this.m_aColumns = Array.isArray(aColumns) ? aColumns.map(column_ => {
         return column_ instanceof column ? column_ : new column(column_);
      }) : [];
   }
   
   // Getters and setters
   get length() { return this.m_aColumns.length; }
   
   // Access methods
   get_column(column_) {
      if(typeof column_ === 'number') {
         // Find by index
         return this.m_aColumns[column_] || null;
      } else if(typeof column_ === 'string') {
         // Find by name or alias
         return this.m_aColumns.find(c_ => c_.m_sName === column_ || c_.m_sAlias === column_ ) || null;
      }
      return null;
   }
   
   // Get column by name
   get_column_by_name(sName) {
      const column_ = this.m_aColumns.find(c_ => c_.m_sName === sName) || null;                    console.assert(column_ !== null, `Column with name "${sName}" not found`);
      return column_;
   }

   // Get column by alias
   get_column_by_alias(sAlias) {
      const column_ = this.m_aColumns.find(c_ => c_.m_sAlias === sAlias) || null;                  console.assert(column_ !== null, `Column with alias "${sAlias}" not found`);
      return column_;
   }   

   get_column_index(column_) {
      if(typeof column_ === 'string') {
         // Find index by name or alias
         return this.m_aColumns.findIndex(c_ => c_.m_sName === column_ || c_.m_sAlias === column_ );
      } 
      else if(column_ instanceof column) {
         // Find index of column object
         return this.m_aColumns.findIndex(c_ => c_.m_sName === column_.m_sName && c_.m_sAlias === column_.m_sAlias ); 
      }

      return -1;
   }
   
   // ## Operations

   add(column_, sType, sAlign) {
      const columnAdd = column_ instanceof column ? column_ : new column(column_, sType, sAlign);
      this.m_aColumns.push(columnAdd);
      return this;
   }
   
   remove(column_) {
      const iIndex = this.get_column_index(column_);
      if(iIndex !== -1) { this.m_aColumns.splice(iIndex, 1); }
      return this;
   }
   
   move(column_, iNewIndex) {
      const iCurrentIndex = this.get_column_index(column_);
      if(iCurrentIndex !== -1 && iNewIndex >= 0 && iNewIndex < this.m_aColumns.length) {
         const [c_] = this.m_aColumns.splice(iCurrentIndex, 1);
         this.m_aColumns.splice(iNewIndex, 0, c_);
      }
      return this;
   }
   
   // ## Filter and map methods

   // Filter columns by type, like name, alias, type, align, sorted, width, hide
   // and return an array of member values


   /**
    * @brief Get attributes of columns based on the specified type.
    * 
    * This method retrieves the attributes of columns based on the specified type.
    * It can return the values of specific attributes or all attributes of the columns.
    * And values are returned as an array with the same order as the columns and type as member values.
    * 
    * @param {string} sType type to filter by (e.g., "name", "alias", "type", "align", "sorted", "width", "hide", "all").
    * @param {any} bAll - If true, return all attributes of the columns. Default is false and then only visible columns are returned.
    * @returns {Array} An array of values corresponding to the specified type.
    */
   attributes( sType, bAll = false ) {
      let aColumns = this.m_aColumns;
      sType = sType.toLowerCase();
      if(bAll !== true) {
         /// take only visible columns
         aColumns = aColumns.filter(col => !col.m_bHide);
      }

      // ## Filter by type, like name, alias, type, align, sorted, width, hide
      if(sType === "name") { return aColumns.map(col => col.m_sName); } 
      else if(sType === "alias") { return aColumns.map(col => col.m_sAlias); } 
      else if(sType === "type") { return aColumns.map(col => col.m_sType); } 
      else if(sType === "align") { return aColumns.map(col => col.m_sAlign); } 
      else if(sType === "sorted") { return aColumns.map(col => col.m_sSorted);  } 
      else if(sType === "width") { return aColumns.map(col => col.m_iWidth); } 
      else if(sType === "hide") { return aColumns.map(col => col.m_bHide); }
      else if(sType === "all") {
         return aColumns.map(col => {
            return { name: col.m_sName, alias: col.m_sAlias, type: col.m_sType, align: col.m_sAlign, sorted: col.m_sSorted, width: col.m_iWidth, hide: col.m_bHide };
         });
      } else {
         console.error(`Invalid type "${sType}" for attributes() method`);
         return [];
      }
   }

   // setter for all attributes based on the specified type
   attributes_set( sType, v_ ) {
      if( Array.isArray(v_) === true ) {
         let aValues = v_;
         sType = sType.toLowerCase();
         if(sType === "name") { this.m_aColumns.forEach((c_, iIndex) => c_.m_sName = aValues[iIndex]); }
         else if(sType === "alias") { this.m_aColumns.forEach((c_, iIndex) => c_.m_sAlias = aValues[iIndex]); }
         else if(sType === "type") { this.m_aColumns.forEach((c_, iIndex) => c_.m_sType = aValues[iIndex]); }
         else if(sType === "align") { this.m_aColumns.forEach((c_, iIndex) => c_.m_sAlign = aValues[iIndex]); }
         else if(sType === "sorted") { this.m_aColumns.forEach((c_, iIndex) => c_.m_sSorted = aValues[iIndex]); }
         else if(sType === "width") { this.m_aColumns.forEach((c_, iIndex) => c_.m_iWidth = aValues[iIndex]); }
         else if(sType === "hide") { this.m_aColumns.forEach((c_, iIndex) => c_.m_bHide = aValues[iIndex]); }
         else {
            console.error(`Invalid type "${sType}" for attributes() method`);
            return [];
         }
      }
      else {
         // ## Set all values to same single value
         sType = sType.toLowerCase();
         if(sType === "name") { this.m_aColumns.forEach(c_ => c_.m_sName = v_); }
         else if(sType === "alias") { this.m_aColumns.forEach(c_ => c_.m_sAlias = v_); }
         else if(sType === "type") { this.m_aColumns.forEach(c_ => c_.m_sType = v_); }
         else if(sType === "align") { this.m_aColumns.forEach(c_ => c_.m_sAlign = v_); }
         else if(sType === "sorted") { this.m_aColumns.forEach(c_ => c_.m_sSorted = v_); }
         else if(sType === "width") { this.m_aColumns.forEach(c_ => c_.m_iWidth = v_); }
         else if(sType === "hide") { this.m_aColumns.forEach(c_ => c_.m_bHide = v_); }
         else {
            console.error(`Invalid type "${sType}" for attributes() method`);
            return [];
         }
      }
   }

   // Get visible columns (column objects) (not hidden)
   get_visible_columns() { return new columns(this.m_aColumns.filter(col => !col.m_bHide)); }
   
   // Get column names as array
   get_column_names() { return this.m_aColumns.map(column_ => column_.m_sName); }
   
   // Get the display names of the columns (alias or name) in array
   get_column_display_names() { return this.m_aColumns.map(column_ => column_.get_display_name()); }

   // Get types of columns as array
   get_column_types() { return this.m_aColumns.map(column_ => column_.m_sType); }

   // Get alignments of columns as array
   get_column_aligns() { return this.m_aColumns.map(column_ => column_.m_sAlign); }
   
   // ## Sort methods

   sort_by_name(bAscending = true) {
      this.m_aColumns.sort((a, b) => {
         return bAscending ? 
            a.m_sName.localeCompare(b.m_sName) : 
            b.m_sName.localeCompare(a.m_sName);
      });
      return this;
   }
   
   // ## Utility methods

   for_each(callback_) {
      this.m_aColumns.forEach(callback_);
      return this;
   }
   
   map(callback_) {return this.m_aColumns.map(callback_); }
   
   clone() { return new columns(this.m_aColumns.map(col => col.clone())); }
   
   to_array() { return [...this.m_aColumns]; }

   // ## Debugging

   // Assert method to validate internal state
   assert_valid() {
      console.assert(Array.isArray(this.m_aColumns), "Invalid state: m_aColumns must be an array");
      this.m_aColumns.forEach((c_, iIndex) => {
         console.assert(c_ instanceof column, `Invalid column at index ${iIndex}: must be an instance of column`);
         c_.assert_valid(); // Validate each column using its own assert_valid method
      });
   }
}