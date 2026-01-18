/**
 * Table - A simple data table class for managing tabular data with typed columns.
 *
 * **Quick Start:**
 * ```javascript
 * // Create table with column definitions
 * const table = new Table(["Name", "Age", "Email"]);
 *
 * // Add data rows
 * table.Add(["John", 30, "john@example.com"]);
 * table.Add(["Jane", 25, "jane@example.com"]);
 *
 * // Get all data with headers
 * const data = table.GetData();
 *
 * // Or create from 2D array
 * const table2 = new Table([["Name", "Age"], ["John", 30]]);
 * table2.PrepareColumns(); // Auto-generate columns from first row
 * ```
 *
 * **Key Concepts:**
 * - **Columns:** Define structure (name, type, alias, alignment). Access via name or index.
 * - **Rows:** Array of cell values matching column order.
 * - **Cell Access:** Use `GetCellValue(row, column)` or `SetCellValue(row, column, value)`.
 *
 * **Common Methods:**
 * - `GetData(options)` - Retrieve data with optional sorting, headers, or index column
 * - `Add(row)` - Add single row or multiple rows
 * - `Delete(position, length)` - Remove rows
 * - `GetColumnIndex(name)` - Find column by name or alias
 */
class Table {

   static column = class {
      /**
       * @param {Object|string} options_ - Either a configuration object or the column name string
       * @param {string} options_.sName
       * @param {string} [options_.sAlias]
       * @param {string} [options_.sType="unknown"]
       * @param {number} [options_.iState=0]
       * @param {number} [options_.iSpecificType=0]
       */
      constructor(options_ = {}) {
         if( typeof options_ === "string" ) { options_ = {sName: options_}; }
         if( typeof options_ !== "object" ) { throw new Error("Invalid argument"); }

         const oOptions = Object.assign({sName: "",  sAlias: "",  sType: "string", iState: 0, iSpecificType: 0 }, options_);

         this.sName = oOptions.sName || "";
         this.sAlias = oOptions.sAlias || this.sName;
         this.sType = oOptions.sType || "unknown";
         this.iState = oOptions.iState || 0; // e.g., 0: none, 1: sorted asc, 2: sorted desc, 4: aligned middle, 8: aligned right
         this.iSpecificType = oOptions.iSpecificType || 0;
      }

      is_string() { return this.sType === "string"; }
      is_number() { return this.sType === "number"; }
      is_aligned_middle() { return (this.iState & 4) === 4; }
      is_aligned_right() { return (this.iState & 8) === 8; }

      get name() { return this.sName; }
      get alias() { return this.sAlias; }
      get type() { return this.sType; }
   }


   // @API [tag: table, construction] [summary: create table]

   /** -----------------------------------------------------------------------
    * Constructor
    *
    * Arguments passed to constructor is made as flexible as possible.
    * If you pass first value in with the format [[]] it is table data.
    * If you pass first value in with the format [] it is column data.
    *
    * @param {Object} options_
    * @param {Array<Array>} [options_.aTable=[]]
    * @param {Array<string>} [options_.aColumn=[]]
    */
   constructor(columns_ = [], options_ = {}) {
      if( typeof columns_ === "string" ) {
         columns_ = columns_.split(",")
      }
      if( !Array.isArray(columns_) ) { throw new Error("Invalid argument"); }

      // Check if the first element is an array to identify a 2D table structure
      const bIsTableData = columns_.length > 0 && Array.isArray(columns_[0]);

      if( bIsTableData ) {
         this.aTable = columns_;
         this.aColumn = [];
      }
      else {
         // ## If not table data, assume it's column data
         const aColumn = Array.isArray( columns_ ) ? columns_.map(column => new Table.column(column)) : [];
         this.aTable = options_.aTable || [];
         this.aColumn = options_.aColumn || aColumn;
      }
   }

   /** -----------------------------------------------------------------------
    * Get column index for name or alias
    *
    * Returns index to column or -1 if not found based on name or alias.
    *
    * @param {string|Object} column_ if string then match name or alias, if object then pick type and match
    * @returns {number} index to column or -1 if not found
    */
   GetColumnIndex( column_ ) {
      if( typeof column_ === "string") {
         // ## First pass: check by name .....................................
         for(let i = 0; i < this.aColumn.length; i++) {
            if(this.aColumn[i].sName === column_) { return i; }
         }

         // ## Second pass: check by alias if name not found .................
         for(let i = 0; i < this.aColumn.length; i++) {
            if(this.aColumn[i].sAlias === column_) { return i; }
         }
         return -1;
      }
      if( typeof column_ === "object") {
         // ## If sName .....................................
         if( column_.sName ) {
            for(let i = 0; i < this.aColumn.length; i++) { if(this.aColumn[i].sName === column_.sName) { return i; } }
         }
         // ## If sAlias .....................................
         if( column_.sAlias ) {
            for(let i = 0; i < this.aColumn.length; i++) { if(this.aColumn[i].sAlias === column_.sAlias) { return i; } }
         }

         return -1;
      }

      return column_;
   }

   // Return column count ----------------------------------------------------
   GetColumnCount() { return this.aColumn.length; }

   // Return type for column -------------------------------------------------
   GetColumnType(iColumn) {
      if(iColumn < 0 || iColumn >= this.aColumn.length) { return null; }
      return this.aColumn[iColumn].sType;
   }

   /** -----------------------------------------------------------------------
    * Get header row use alias or name in columns and generates a row that is returned
    */
    GetHeaderRow() {
       return this.aColumn.map(column => column.sAlias || column.sName);
    }


   /** -----------------------------------------------------------------------
    * Returns array with table data
    * @param {Object|string} options_ Configuration options or a string shorthand
    * @param {boolean} [options_.bHeader=true] Include header row at the beginning
    * @param {boolean} [options_.bIndex=false] Include row index as first column
    * @param {number} [options_.iSort=0] Column index to sort by (positive for ascending,
    *                                    negative for descending, 0 for no sorting).
    *                                    Index adjusts based on bIndex option.
    * @param {Array<number>} [options_.aRows] Array of row indices to include
    * @param {Array<number>} [options_.aColumn] Array of column indices to include
    * @returns {Array<Array>} 2D array of table data
    * @example
    * // Get all data with header (default)
    * const data = table.GetData();
    *
    * @example
    * // String shorthand: include index column
    * const data = table.GetData('index');
    *
    * @example
    * // String shorthand: include both index and header
    * const data = table.GetData('index header');
    *
    * @example
    * // String shorthand: include both index and header, and selected rows
    * const data = table.GetData({ aRows: [0, 2, 4] });
    *
    * @example
    * // Object options: sort by column 2 descending, no header
    * const data = table.GetData({
    *   bHeader: false,
    *   bIndex: true,
    *   iSort: -2  // Sorts by second column descending
    * });
    */
   GetData(options_) {
      if( typeof options_ === 'string' ) {
         let o = {};
         const s_ = options_;
         if( s_.indexOf('index') !== -1 ) { o.bIndex = true; }
         if( s_.indexOf('header') !== -1 ) { o.bHeader = true; }
         options_ = o;
      }

      const oOptions = Object.assign({ bHeader: true, iSort: 0, bIndex: false, aRows: null, aColumn: null }, options_); // retrieval configuration
      let aData = []; // Generated data that is returned

      // ## Build table data to return  .......................................
      if( oOptions.bIndex === false ) {                                       // No index, be aware about how to access row data
         for(let i = 0; i < this.aTable.length; i++) { aData.push(this.GetRow(i)); }
      }
      else if( oOptions.bIndex === true ) {                                   // Add index to row
         for(let i = 0; i < this.aTable.length; i++) {
            let aRow = [i];
            aRow = aRow.concat(this.GetRow(i))
            aData.push( aRow );
         }
      }

      // ## If selected rows then extract rows into new array and use that ...
      if( oOptions.aRows !== null ) {
         aData = oOptions.aRows.map(iRow => aData[iRow]);
      }

      // ## If selected columns then extract columns from each row ...............
      if( oOptions.aColumn !== null ) {
         aData = aData.map(aRow => {
            return oOptions.aColumn.map(iCol => aRow[iCol]);
         });
      }

      // ## Sorting ...........................................................
      const iFirstColumn = oOptions.bIndex === true ? 1 : 0;                  // If index then first column is at 1
      const bIsString = this.GetColumnType(iFirstColumn) === 'string';        // Check if column type is string
      if(oOptions.iSort > 0) {                                                // ascending order
         const iSort = oOptions.iSort - iFirstColumn;
         aData.sort((a, b) => {
            if(bIsString) {
               const a_ = a[iSort];
               const b_ = b[iSort];

               // ## Handle null/undefined values for strings .................
               if (a_ == null && bIsString == null) return 0;
               if (a_ == null) return -1;                                     // null/undefined comes before strings
               if (b_ == null) return 1;                                      // null/undefined comes before strings

               return String(a_).localeCompare(String(b_));
            }
            return a[iSort] - b[iSort];
         });
      }

      if(oOptions.iSort < 0) {                                                // descending order
         const iSort = Math.abs(oOptions.iSort) - iFirstColumn;
         aData.sort((a, b) => {
            if(bIsString) {
               const a_ = a[iSort];
               const b_ = b[iSort];

               // ## Handle null/undefined values for strings (descending order)
               if (a_ == null && b_ == null) return 0;
               if (a_ == null) return 1;                                      // null/undefined comes after strings in descending
               if (b_ == null) return -1;                                     // null/undefined comes after strings in descending

               return String(b_).localeCompare(String(a_));
            }
            return b[iSort] - a[iSort];
         });
      }

      // ## Add header row if specified ......................................
      if(oOptions.bHeader) {
         let aHeader = this.GetHeaderRow();
         if(oOptions.aColumn !== null) { aHeader = oOptions.aColumn.map(iColumn => aHeader[iColumn]); } // Filter header columns
         if(oOptions.bIndex) { aHeader = ["#", ...aHeader]; }                 // Add index column if specified
         aData.unshift(aHeader);
      }

      return aData;
   }

   /** -----------------------------------------------------------------------
    * Find rows based on find condition
    * @param {Object|Function} find_ Find condition configuration or callback function
    * @param {Function} [find_.callback] Callback function that receives (aRow, iIndex) and returns boolean
    * @param {*} [find_.value] Value or array of values to match in any column
    * @param {number|Array<number>} [find_.iColumn] Column index or array of column indices to search in
    * @returns {Array<number>} Array of matching row indices
    * @example
    * // Find rows using callback
    * const aRows = table.FindAll((aRow, iIndex) => {
    *    return aRow[0] > 100 && aRow[1] === 'active';
    * });
    *
    * @example
    * // Find rows with value 'active' in any column
    * const aRows = table.FindAll({ value: 'active' });
    *
    * @example
    * // Find rows with value 'active' in column 2
    * const aRows = table.FindAll({ value: 'active', iColumn: 2 });
    *
    * @example
    * // Find rows with value 100 or 200 in column 0
    * const aRows = table.FindAll({ value: [100, 200], iColumn: 0 });
    *
    * @example
    * // Find rows with value 'error' in columns 1 or 3
    * const aRows = table.FindAll({ value: 'error', iColumn: [1, 3] });
    *
    * @example
    * // Find rows with value 'error' in columns 1 or 3 using callback directly
    * const aRows = table.FindAll((aRow, iIndex) => {
    *    return aRow[1] === 'error' || aRow[3] === 'error';
    * });
    */
   FindAll(find_) {                                                                                console.assert( typeof find_ === 'function' || (typeof find_ === 'object' && find_ !== null), "FindAll: Invalid find condition. Expected a function or an object.");
      const aFind = [];

      // ## Handle callback function directly ...................................
      if( typeof find_ === 'function' ) {
         for( let iRow = 0; iRow < this.aTable.length; iRow++ ) {
            const aRow = this.GetRow(iRow);
            if( find_(aRow, iRow) === true ) {
               aFind.push(iRow);
            }
         }
         return aFind;
      }

      // ## Handle object-based find conditions ..............................
      const bHasCallback = typeof find_.callback === 'function';
      const bHasValue = 'value' in find_;
      const bHasColumn = 'iColumn' in find_;

      // ## Callback-based search ..........................................
      if( bHasCallback ) {
         for( let iRow = 0; iRow < this.aTable.length; iRow++ ) {
            const aRow = this.GetRow(iRow);
            if( find_.callback(aRow, iRow) === true ) {
               aFind.push(iRow);
            }
         }
         return aFind;
      }

      // ## Value-based search ............................................
      if( bHasValue ) {
         const aValues = Array.isArray(find_.value) ? find_.value : [find_.value]; // Prepare values array

         // Prepare columns array (all columns if not specified)
         let aColumns = [];
         if( bHasColumn ) {
            aColumns = Array.isArray(find_.iColumn) ? find_.iColumn : [find_.iColumn];
         }
         else {
            for( let iColumn = 0; iColumn < this.aColumn.length; iColumn++ ) { aColumns.push(iColumn); }
         }

         // ### Search through rows ..........................................
         for( let iRow = 0; iRow < this.aTable.length; iRow++ ) {
            const aRow = this.GetRow(iRow);
            let bMatch = false; // No match yet

            // #### Check if any column matches any value
            for( let i = 0; i < aColumns.length; i++ ) {
               const iColumn = aColumns[i];
               const vCell = aRow[iColumn];

               for( let j = 0; j < aValues.length; j++ ) {
                  const vValue = aValues[j];

                  if( vCell == vValue ) { bMatch = true; break; }             // Compare values (handle type coercion for loose matching)
               }
               if( bMatch ) break;
            }

            if( bMatch ) { aFind.push(iRow); }                                // keep found row index
         }
      } // if( bHasValue ) {

      return aFind;
   }

   // Convert row data to an object ------------------------------------------
   AsObject(iRow) {
      const o = {};
      for(let iColumn = 0; iColumn < this.aColumn.length; iColumn++) {
         o[this.aColumn[iColumn].name] = this._GetCellValue(iRow, iColumn);
      }
      return o;
   }

   // Convert row data to a string -------------------------------------------
   AsString(iRow, sSeparator = "\t") {
      let s = "";
      const iColumnCount = this.aColumn.length;
      for(let iColumn = 0; iColumn < iColumnCount; iColumn++) {
         s += this._GetCellValue(iRow, iColumn) + sSeparator;
      }
      return s.trim();
   }

   /** -----------------------------------------------------------------------
    * Escape XML special characters in a value
    * @param {any} value_ the value to escape
    * @returns {string} the escaped value
    */
   _EscapeXmlValue(value_) {
      if( value_ === null || value_ === undefined ) { return ""; }
      let sValue = String(value_);
      // Replace XML special characters with their entity references
      sValue = sValue.replace(/&/g, "&amp;");
      sValue = sValue.replace(/</g, "&lt;");
      sValue = sValue.replace(/>/g, "&gt;");
      sValue = sValue.replace(/"/g, "&quot;");
      sValue = sValue.replace(/'/g, "&apos;");
      return sValue;
   }

   /** -----------------------------------------------------------------------
    * Convert a row's data to an XML string
    *
    * Generates XML in one of two formats depending on the options provided.
    * Columns with null or undefined values are automatically skipped.
    * XML special characters in values are automatically escaped.
    *
    * @param {number} iRow - The row index to convert to XML
    * @param {Object} oOptions - Configuration options for XML generation
    * @param {string} [oOptions.sRow="row"] - The name for the row element
    * @param {string} [oOptions.sColumn="column"] - The name for column elements (used when bAttribute=false)
    * @param {boolean} [oOptions.bAttribute=false] - If false, values are in elements; if true, values are in attributes
    * @returns {string} The XML string representation of the row data
    *
    * @example
    * // Elements mode (default): <row><column name="name">John</column><column name="age">30</column></row>
    * const xml1 = table.AsXml(0);
    *
    * @example
    * // Attributes mode: <row name="John" age="30" />
    * const xml2 = table.AsXml(0, { bAttribute: true });
    *
    * @example
    * // Custom element names
    * const xml3 = table.AsXml(0, { sRow: "person", sColumn: "field" });
    * // Result: <person><field name="name">John</field><field name="age">30</field></person>
    */
   AsXml(iRow, oOptions = {}) {
      let sResult = "";
      const sRow = oOptions.sRow || "row";
      const sColumn = oOptions.sColumn || "column";
      const bAttribute = oOptions.bAttribute || false;

      if( bAttribute == false ) {
         // ## Generate XML with values in elements, structure is <sRow><sColumn name="name">value</sColumn></sRow>
         sResult += `<${sRow}>`;
         const iColumnCount = this.GetColumnCount();
         for(let iColumn = 0; iColumn < iColumnCount; iColumn++) {
            const v_ = this._GetCellValue(iRow, iColumn);
            if( v_ !== null && v_ !== undefined ) {
               sResult += `<${sColumn} name="${this._EscapeXmlValue(this.aColumn[iColumn].name)}">${this._EscapeXmlValue(v_)}</${sColumn}>`;
            }
         }
         sResult += `</${sRow}>`;
      }
      else if( bAttribute == true ) {
         // ## Generate XML with values in attributes, structure is <sRow sColumn1="value1" sColumn2="value2" />
         sResult += `<${sRow}`;
         const iColumnCount = this.GetColumnCount();
         for(let iColumn = 0; iColumn < iColumnCount; iColumn++) {
            const v_ = this._GetCellValue(iRow, iColumn);
            if( v_ !== null && v_ !== undefined ) {
               sResult += ` ${this._EscapeXmlValue(this.aColumn[iColumn].name)}="${this._EscapeXmlValue(v_)}"`;
            }
         }
         sResult += ` />`;
      }

      return sResult;
   }

   AsJson(row_, oOptions = {}) {
      const bIncludeNull = oOptions.bIncludeNull || false;
      const iIndent = oOptions.iIndent || 3;

      const iRowBegin = row_ || 0;
      const iRowEnd = iRowBegin + ( row_ ? 1 : this.Size() );

      let aRows = [];

      for(let iRow = iRowBegin; iRow < iRowEnd; iRow++) {
         const o = {};
         for(let iColumn = 0; iColumn < this.aColumn.length; iColumn++) {
            const v_ = this._GetCellValue(iRow, iColumn);
            if( bIncludeNull || (v_ !== null && v_ !== undefined) ) {
               o[this.aColumn[iColumn].name] = v_;
            }
         }
         aRows.push(o);
      }

      return JSON.stringify(aRows, null, iIndent);
   }

   // Get internal table data array ------------------------------------------
   Data() { return this.aTable; }

   /** -----------------------------------------------------------------------
    * Get cell value
    * @param {number} iRow index for row
    * @param {number | string} column_ index for column or column name
    */
   GetCellValue(iRow, column_) {
      let iColumn = column_;
      if( typeof column_ === "string") { iColumn = this.GetColumnIndex(column_); }
      if(iRow < 0 || iRow >= this.aTable.length || iColumn < 0 || iColumn >= this.aColumn.length) {
         return null;
      }

      return this._GetCellValue(iRow, iColumn);
   }

   /** -----------------------------------------------------------------------
    * Get cell value
    * Internal method to get cell value, no checks for valid column or row
    * @param {number} iRow index for row
    * @param {number} iColumn index for column
    */
   _GetCellValue(iRow, iColumn) {
      let value_ = this.aTable[iRow][iColumn]; // Get raw cell value from cell position
      if(Array.isArray(value_)) { value_ = value_[0];  }                      // if column is array, return first element

      return value_;
   }

   /** -----------------------------------------------------------------------
    * Set cell value
    * @param {number} iRow
    * @param {number|string} column_ index or name for column values is set to
    * @param {any} value_ value set to cell
    */
   SetCellValue(iRow, column_, value_) {
      let iColumn = column_;
      if( typeof column_ === "string") { iColumn = this.GetColumnIndex(column_); }
      if(iRow < 0 || iRow >= this.aTable.length || iColumn < 0 || iColumn >= this.aColumn.length) { return false; }

      this._SetCellValue(iRow, iColumn, value_);
      return true;
   }

   /** -----------------------------------------------------------------------
    * Internal method to set cell value, no checks for valid column or row
    * @param {number} iRow index for row
    * @param {number} iColumn index for column
    * @param {any} value_ value set to cell
    */
   _SetCellValue(iRow, iColumn, value_) { this.aTable[iRow][iColumn] = value_; }

   /** -----------------------------------------------------------------------
    * Get row data
    * @param {number} iRow index for row
    * @returns {Array<any>} Array of cell values for the row
    */
    GetRow(iRow) {
      if(iRow < 0 || iRow >= this.aTable.length) { return null; }

      return this._GetRow(iRow);
   }

   /** -----------------------------------------------------------------------
    * Internal method to get row data, no checks for valid row
    * @param {number} iRow index for row
    * @returns {Array<any>} Array of cell values for the row
    */
   _GetRow(iRow) {
     const row_ = [];
     for(let iColumn = 0; iColumn < this.aColumn.length; iColumn++) {  row_.push(this._GetCellValue(iRow, iColumn)); }

     return row_;
  }


   // Check if table is empty ------------------------------------------------
   Empty() { return this.aTable.length === 0; }

   // Return number of rows --------------------------------------------------
   Size() {  return this.aTable.length; }

   /** -----------------------------------------------------------------------
    * Add rows to the table
    *
    * Adding rows as string needs a splitter, default is ","
    * If object is passed the key is matched to the column name
    *
    * @param {Object |Array | string} table_ - Data to add (string, row array, or array of rows)
    * @param {string} sSeperator - Optional separator for string input (default: ",")
    */
   Add(table_, sSeperator) {
      let aTable = table_;
      if(typeof table_ === "string") {
         if(!sSeperator) { sSeperator = "," }                                 // default separator is ","
         aTable = [table_.split(sSeperator)];
      }
      else if(Array.isArray(table_) && table_.every(Array.isArray) == false) { aTable = [table_]; } // check for single [] to add
      else if( Object.prototype.toString.call(table_) === "[object Object]") {
         // ## generate array with the amount of columns table has
         const iColumnCount = this.GetColumnCount();
         aTable = Array(iColumnCount); // Initialize array with undefined values

         // ## Iterate object, find matching column and set value in array ...
         for(const [key_, value_] of Object.entries(table_)) {
            const iColumn = this.GetColumnIndex(key_); // column index for key name
            if(iColumn !== -1) { aTable[iColumn] = value_; }
         }
         aTable = [aTable];                                                   // Wrap the array in another array to match the expected format
      }

      // ## Add rows to internal table .......................................
      for(let i = 0; i < aTable.length; i++) { this.aTable.push(aTable[i]); }
   }

   /** -----------------------------------------------------------------------
    * Deletes one or more rows from the table.
    *
    * @param {number} iPosition - The starting index from which rows will be deleted
    * @param {number} iLength   - Number of rows to delete (defaults to 1)
    */
   Delete(iPosition, iLength) {
      const iRows = this.aTable.length;
      const iMaxLength = iRows - iPosition;

      if(!iLength || iLength === 0) { iLength = 1; }

      iLength = Math.min(iLength, iMaxLength);

      this.aTable.splice(iPosition, iLength);
   }

   // Clears all rows from the table. ----------------------------------------
   Clear() { this.aTable = []; }

   // @API [tag: table, utilities] [description: Helper methods, like utilities for table]

   /** -----------------------------------------------------------------------
    * Parses the types of the data in the table for a specific row and sets the
    * column type if it is not already set. Does not modify the cell values.
    *
    * @param {Array|number} row_ - The row array or the row index to parse types for
    */
   ParseRowTypesToColumns( row_ ) {
      let aRow = row_;
      if( row_ === undefined || row_ === null ) { row_ = 1; } // Default to the secornd row because first may be header
      if( typeof row_ === "number" && this.Size() === 1) { row_ = 0; }

      if( typeof row_ === "number" ) { aRow = this._GetRow(row_); }
      if(!aRow) return;

      for(let i = 0; i < aRow.length; i++) {
         const v_ = aRow[i]; // Get the value at the current index (i = column)

         if(v_ === null || v_ === undefined || v_ === "") { continue; }       // Ignore empty or null values as they don't help determine type

         if(this.aColumn[i] && (this.aColumn[i].sType === "unknown" || !this.aColumn[i].sType)) { // Only proceed if the column type is currently unknown

            // ## Determine Type ..............................................
            let sDetectedType = typeof v_;

            if(sDetectedType === "string") {                                  // If the value is a string, check if it represents a number
               const n_ = Number(v_);
               if(!isNaN(n_)) { sDetectedType = "number"; }
            }

            // ## Apply to Metadata ...........................................
            this.aColumn[i].sType = sDetectedType;
         }// if(
      }// for(
   }

   /** -----------------------------------------------------------------------
    * Prepares column definitions from the internal row data.
    * Typically called if aTable was passed in without aColumn metadata.
    *
    * @example
    * const table = new Table();
    * table.AddRows([
    *    ["Name", "Age", "City"],
    *    ["Alice", "30", "New York"],
    *    ["Bob", "25", "Los Angeles"]
    * ]);
    * table.PrepareColumns();
    */
   PrepareColumns() { console.assert(this.aTable.length > 0, "Table is empty");
      const aHeader = this.aTable[0];
      const aColumns = [];

      for(let i = 0; i < aHeader.length; i++) {
         const sName = String(aHeader[i]); // This should be the name of the column if you call this method

         // Peek at next row to guess type
         let sType = "string";
         if(this.aTable.length > 1) {
            const v_ = this._GetCellValue(1, i);
            if(typeof v_ === "number") sType = "number";
            else if(v_ instanceof Date) sType = "date";
         }

         aColumns.push(new Table.column({ sName: sName, sType: sType }));
      }

      this.aColumn = aColumns;
      return this.aColumn;
   }

}
