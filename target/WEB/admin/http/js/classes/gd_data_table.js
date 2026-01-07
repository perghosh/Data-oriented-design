class Table
{
    /**
     * Constructor
     * @param {Object} options_
     * @param {Array<Array>} [options_.aTable=[]]
     * @param {Array<string>} [options_.aColumn=[]]
     */
    constructor(options_ = {})
    {
        const oOptions = Object.assign({
            aTable: [],
            aColumn: []
        }, options_);

        this.aTable = oOptions.aTable;
        this.aColumn = oOptions.aColumn;
    }

    /**
     * Get table data
     * @param {Object} options_
     * @param {boolean} bColumn 
     * @param {number} iSort
     */
    GetData( options_ )
    {
        const oOptions = Object.assign({
            bColumn: true,
            iSort: 0
        }, options_);

        let aData = [];

        if(oOptions.bColumn == true) { aData.push(...this.aColumn) }
        aData.push(...this.aTable);

        return aData;
    }

    /**
     * Add rows to the table
     * @param {Array | string} table_ - Data to add (string, row array, or array of rows)
     * @param {string} sSeperator - Optional separator for string input (default: ",")
     */
    Add(table_, sSeperator)
    {
        let aTable = table_;
        if(typeof table_ === "string")
        {
            if(!sSeperator) {sSeperator = ","}
            aTable = [table_.split(sSeperator)];   
        }
        else if(Array.isArray(table_) && table_.every(Array.isArray) == false)
        {
            aTable = [table_];
        }

        for(let i = 0; i < aTable.length; i++) // undefined
        {
            this.aTable.push(aTable[i]);
        }
        console.log(this.aTable);
    }
    
    /**
     * Deletes one or more rows from the table.
     *
     * @param {number} iPosition - The starting index from which rows will be deleted
     * @param {number} iLength   - Number of rows to delete (defaults to 1)
     */
    Delete(iPosition, iLength)
    {
        const iRows = this.aTable.length;
        const iMaxLength = iRows - iPosition;

        if( !iLength || iLength === 0 ) { iLength = 1; }  

        iLength = Math.min(iLength, iMaxLength);

        this.aTable.splice(iPosition, iLength);
    }
}