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
         this.iState = oOptions.iState || 0; // e.g., 0: none, 1: sorted asc, 2: sorted desc
         this.iSpecificType = oOptions.iSpecificType || 0;
      }

      is_string() { return this.sType === "string"; }
      is_number() { return this.sType === "number"; }
   }

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
    * Get column index by name or alias
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

      const oOptions = Object.assign({ bHeader: true, iSort: 0, bIndex: false }, options_); // retrieval configuration
      let aData = []; // Generated data that is returned

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

      const iFirstColumn = oOptions.bIndex === true ? 1 : 0;                  // If index then first column is at 1

      // Check if column type is string
      const bIsString = this.GetColumnType(iFirstColumn) === 'string';

      if(oOptions.iSort > 0) {
         const iSort = oOptions.iSort - iFirstColumn;
         aData.sort((a, b) => {
            if(bIsString) {
               const a_ = a[iSort];
               const b_ = b[iSort];

               // Handle null/undefined values for strings
               if (a_ == null && bVal == null) return 0;
               if (a_ == null) return -1; // null/undefined comes before strings
               if (b_ == null) return 1;  // null/undefined comes before strings

               return String(a_).localeCompare(String(b_));
            }
            return a[iSort] - b[iSort];
         });
      }

      if(oOptions.iSort < 0) {
         const iSort = Math.abs(oOptions.iSort) - iFirstColumn;
         aData.sort((a, b) => {
            if(bIsString) {
               const a_ = a[iSort];
               const b_ = b[iSort];

               // Handle null/undefined values for strings (descending order)
               if (a_ == null && b_ == null) return 0;
               if (a_ == null) return 1; // null/undefined comes after strings in descending
               if (b_ == null) return -1; // null/undefined comes after strings in descending

               return String(b_).localeCompare(String(a_));
            }
            return b[iSort] - a[iSort];
         });
      }

      if( oOptions.bHeader) { aData.unshift(this.GetHeaderRow()); }           // add header row

      return aData;
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
      let value_ = this.aTable[iRow][iColumn];

      if(Array.isArray(value_)) { value_ = value_[0];  }                      // if column is array, return first element

      return value_;
   }

   /** -----------------------------------------------------------------------
    * Set cell value
    * @param {number} iRow
    * @param {number|string} column_ index or name for column values is set to
    * @param {any} value_ value set to cell
    * @returns
    */
   SetCellValue(iRow, column_, value_) {
      let iColumn = column_;
      if( typeof column_ === "string") { iColumn = this.GetColumnIndex(column_); }
      if(iRow < 0 || iRow >= this.aTable.length || iColumn < 0 || iColumn >= this.aColumn.length) {
         return false;
      }

      this._SetCellValue(iRow, iColumn, value_);
      return true;
   }

   /** -----------------------------------------------------------------------
    * Internal method to set cell value, no checks for valid column or row
    * @param {number} iRow index for row
    * @param {number} iColumn index for column
    * @param {any} value_ value set to cell
    */
   _SetCellValue(iRow, iColumn, value_) {
      this.aTable[iRow][iColumn] = value_;
   }

   /** -----------------------------------------------------------------------
    * Get row data
    * @param {number} iRow index for row
    */
    GetRow(iRow) {
      if(iRow < 0 || iRow >= this.aTable.length) { return null; }

      return this._GetRow(iRow);
   }

   _GetRow(iRow) {
     const row_ = [];
     for(let iColumn = 0; iColumn < this.aColumn.length; iColumn++) {
        row_.push(this._GetCellValue(iRow, iColumn));
     }

     return row_;
  }


   // Check if table is empty ------------------------------------------------
   Empty() { return this.aTable.length === 0; }

   // Return number of rows --------------------------------------------------
   Size() {  return this.aTable.length; }

   /** -----------------------------------------------------------------------
    * Add rows to the table
    * @param {Array | string} table_ - Data to add (string, row array, or array of rows)
    * @param {string} sSeperator - Optional separator for string input (default: ",")
    */
   Add(table_, sSeperator) {
      let aTable = table_;
      if(typeof table_ === "string") {
         if(!sSeperator) { sSeperator = "," }                                 // default separator is ","
         aTable = [table_.split(sSeperator)];
      }
      else if(Array.isArray(table_) && table_.every(Array.isArray) == false) { aTable = [table_]; } // check for single [] to add

      for(let i = 0; i < aTable.length; i++) // undefined
      {
         this.aTable.push(aTable[i]);
      }
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

   // ## Helper methods, like utilities for table

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
         const v_ = aRow[i];

         // Ignore empty or null values as they don't help determine type
         if(v_ === null || v_ === undefined || v_ === "") { continue; }

         // Only proceed if the column type is currently unknown
         if(this.aColumn[i] && (this.aColumn[i].sType === "unknown" || !this.aColumn[i].sType)) {

            // ## Determine Type ..............................................
            let sDetectedType = typeof v_;

            // If the value is a string, check if it represents a number
            if(sDetectedType === "string") {
               const n_ = Number(v_);
               if(!isNaN(n_)) { sDetectedType = "number"; }
            }

            // ## Apply to Metadata ...........................................
            this.aColumn[i].sType = sDetectedType;
         }// if(
      }// for(
   }

   /** -----------------------------------------------------------------------
    * Prepares column definitions from the first row of data.
    * Typically called if aTable was passed in without aColumn metadata.
    */
   PrepareColumns() { console.assert(this.aTable.length > 0, "Table is empty");
      const aHeader = this.aTable[0];
      const aColumns = [];

      for(let i = 0; i < aHeader.length; i++) {
         const sName = String(aHeader[i]);

         // Peek at next row to guess type
         let sType = "string";
         if(this.aTable.length > 1) {
            const vSample = this._GetCellValue(1, i);
            if(typeof vSample === "number") sType = "number";
            else if(vSample instanceof Date) sType = "date";
         }

         aColumns.push(new Table.column({ sName: sName, sType: sType }));
      }

      this.aColumn = aColumns;
      return this.aColumn;
   }

}
