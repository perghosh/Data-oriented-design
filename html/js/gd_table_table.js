import { column, columns } from './gd_table_column.js';


export class table {
   // Define default values as static properties
   static DEFAULT_PAGE_SIZE = 10;

   /**
    * Constructs a Table object to represent tabular data.
    * @param {Array[]|Object[]|null} data - The data to populate the table with.
    * @param {columns|Array|null} columns_ - The columns definition for the table.
    * @constructor
    * @example
    * // Array-based constructor
    * new table([["1", "John"], ["2", "Jane"]], [new column("ID", "integer"), new column("Name")]);
    * // Object-based constructor
    * new table([{ID: "1", Name: "John"}, {ID: "2", Name: "Jane"}], [new column("ID", "integer"), new column("Name")]);
    */
   constructor(data_ = null, columns_ = null) {
      // Initialize with empty data and columns if not provided
      this.m_aData = [];
      this.m_oColumns = columns_ instanceof columns ? columns_ : new columns(columns_ ?? []);

      // Process and load data if provided
      this._init_data(data_);

      this.m_iPageSize = table.DEFAULT_PAGE_SIZE;
   }

   // ## Getters and Setters

   get columns() { return this.m_oColumns; }
   get data() { return [...this.m_aData]; }
   get row_count() { return this.m_aData.length; }
   get page_size() { return this.m_iPageSize; }
   set page_size(iSize) {                                                                          console.assert(iSize > 0, `Invalid page size: ${iSize}`);
      this.m_iPageSize = iSize; 
   }

   // ## Public methods
   /**
    * Removes a row by index.
    * @param {number} index_ - Row index to remove.
    * @returns {Table} This table instance.
    */
   row_remove(index_) {
      if(typeof index_ === 'number') {
         let iRow = index_;
         if(iRow >= 0 && iRow < this.m_aData.length) {
            this.m_aData.splice(iRow, 1);
         }
      }
      return this;
   }

   // ## Utility methods

   /**
    * Returns table data as array of objects.
    * @returns {Array} Array of row objects.
    */
   to_objects() {
      const names = this.m_oColumns.get_column_names();
      return this.m_aData.map(row => 
         Object.fromEntries(row.map((val, i) => [names[i], val]))
      );
   }

   /**
    * Clones the table.
    * @returns {Table} New table instance.
    */
   clone() {
      return new Table([...this.m_aData], this.m_oColumns.clone());
   }


   // ## Debugging

   /**
    * Validates internal state.
    */
   assert_valid() {
      console.assert(Array.isArray(this.m_aData), "Invalid state: m_aData must be an array");
      console.assert(this.m_oColumns instanceof columns, "Invalid state: m_oColumns must be a columns instance");
      this.m_oColumns.assert_valid();
      this._validate_data();
   }

   // ## Private methods

   /**
    * Initializes table data from various input formats.
    * @param {Array|Object} data_ - Input data.
    * @private
    */
   _init_data(data_) {
      if(Array.isArray(data_)) { this._process_array(data_); } 
      else if(typeof data_ === 'object' && data_ !== null) { this._process_object(data_);  }
      this._validate_data();
   }

   /**
    * Processes array-based input data.
    * 
    * This method handles both array of arrays and array of objects. 
    * - `Array`: if the first row is an array of strings and columns are not set, it is treated as column names.
    * - `Object`: If the first row is an array of objects, it is treated as data rows and the keys are removed before adding to the table data.
    * 
    * @param {Array} data_ - Array of arrays or objects.
    * @private
    */
   _process_array(data_) {
      if (data_.length === 0) return;

      if( this.m_oColumns.length === 0 ) {                                     // Check if columns are defined, if not, create them from first row
         const aFirstRow = data_[0];
         // Check if first row contains column names
         if( Array.isArray(aFirstRow) && aFirstRow.every(val => typeof val === 'string') ) {
            this._update_column_names(aFirstRow);
            this.m_aData = data_.slice(1).map(row => [...row]);                // remove first row from data
         }
      }
      else {
         this.m_aData = data_.map(row => Array.isArray(row) ? [...row] : Object.values(row));// clone and convert objects to arrays if needed
      }
   }

   /**
    * Processes object-based input data.
    * @param {Object} data_ - Object with column names as keys.
    * @private
    */
   _process_object(data_) {
      const aColumnNames = this.m_oColumns.get_column_names();
      if (Object.keys(data_).length === 0) return;

      // If columns are not defined, create from object keys
      if (this.m_oColumns.length === 0) {
         Object.keys(data_).forEach(name => {
            this.m_oColumns.add({ name, alias: name });
         });
      }

      // Convert object data to array format
      this.m_aData = Object.values(data_).map(row => 
         aColumnNames.map(name => row[name] ?? null)
      );
   }

   /**
    * Updates column names from first row if applicable.
    * @param {Array} aFirstRow - First row containing column names.
    * @private
    */
   _update_column_names(aFirstRow) {
      aFirstRow.forEach((sName, index) => {
         if(index < this.m_oColumns.length) {
            const column_ = this.m_oColumns.get_column(index);
            if(column_) column_.name = sName;
         } 
         else {
            this.m_oColumns.add({ name: sName, alias: sName });
         }
      });
   }

   /**
    * Validates internal data consistency.
    * @private
    */
   _validate_data() {
      const iExpectedLength = this.m_oColumns.length;
      this.m_aData.forEach((row, iIndex) => {
         console.assert(
            Array.isArray(row) && row.length === iExpectedLength, `Invalid row at index ${iIndex}: expected ${iExpectedLength} columns, got ${row.length}`
         );
      });
   }
}