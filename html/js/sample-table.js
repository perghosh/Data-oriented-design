// Column class to store column metadata and behavior
class Column {
  constructor(name, type, align = 'left', isSorted = false, isHidden = false) {
    this.name = name; // e.g., "ID", "Name"
    this.type = type; // e.g., "number", "string", "date"
    this.align = align; // e.g., "left", "center", "right"
    this.isSorted = isSorted; // boolean
    this.isHidden = isHidden; // boolean
  }

  // Example method for formatting based on type
  formatValue(value) {
    if (this.type === 'date' && value instanceof Date) {
      return value.toLocaleDateString();
    }
    return value;
  }

  // Example method to toggle sorting
  toggleSort() {
    this.isSorted = !this.isSorted;
  }
}

// Table class with extended functionality
class Table {
  constructor(data = [], columns = []) {
    this.data = data; // Array of row arrays, e.g., [[1, "Alice"], [2, "Bob"]]
    this.columns = columns; // Array of Column objects
  }

  // Add a new column
  addColumn(name, type, align = 'left', isSorted = false, isHidden = false) {
    this.columns.push(new Column(name, type, align, isSorted, isHidden));
  }

  // Add a new row
  addRow(row) {
    if (row.length !== this.columns.length) {
      throw new Error(`Row length (${row.length}) must match column count (${this.columns.length})`);
    }
    this.data.push([...row]); // Clone to avoid modifying external data
  }

  // Remove a row by index
  removeRow(rowIndex) {
    if (rowIndex < 0 || rowIndex >= this.data.length) {
      throw new Error(`Invalid row index: ${rowIndex}`);
    }
    this.data.splice(rowIndex, 1);
  }

  // Get cell value by row and column index
  getCellValue(rowIndex, colIndex) {
    if (rowIndex < 0 || rowIndex >= this.data.length) {
      throw new Error(`Invalid row index: ${rowIndex}`);
    }
    if (colIndex < 0 || colIndex >= this.columns.length) {
      throw new Error(`Invalid column index: ${colIndex}`);
    }
    return this.data[rowIndex][colIndex];
  }

  // Set cell value by row and column index
  setCellValue(rowIndex, colIndex, value) {
    if (rowIndex < 0 || rowIndex >= this.data.length) {
      throw new Error(`Invalid row index: ${rowIndex}`);
    }
    if (colIndex < 0 || colIndex >= this.columns.length) {
      throw new Error(`Invalid column index: ${colIndex}`);
    }
    this.data[rowIndex][colIndex] = value;
  }

  // Get column by name
  getColumnByName(name) {
    const column = this.columns.find(col => col.name === name);
    if (!column) {
      throw new Error(`Column not found: ${name}`);
    }
    return column;
  }

  // Get column index by name
  getColumnIndexByName(name) {
    const index = this.columns.findIndex(col => col.name === name);
    if (index === -1) {
      throw new Error(`Column not found: ${name}`);
    }
    return index;
  }

  // Get cell value by row index and column name
  getCellValueByName(rowIndex, colName) {
    const colIndex = this.getColumnIndexByName(colName);
    return this.getCellValue(rowIndex, colIndex);
  }

  // Set cell value by row index and column name
  setCellValueByName(rowIndex, colName, value) {
    const colIndex = this.getColumnIndexByName(colName);
    this.setCellValue(rowIndex, colIndex, value);
  }

  // Get visible data (excluding hidden columns)
  getVisibleData() {
    return this.data.map(row => 
      row.filter((_, index) => !this.columns[index].isHidden)
    );
  }
}

// Example usage
const table = new Table(
  [
    [1, "Alice", new Date("2023-01-01")],
    [2, "Bob", new Date("2023-02-01")]
  ],
  [
    new Column("ID", "number", "center"),
    new Column("Name", "string", "left"),
    new Column("Date", "date", "right", false, true) // Hidden
  ]
);

// Test adding and removing rows
console.log("Initial data:", table.data);
table.addRow([3, "Charlie", new Date("2023-03-01")]);
console.log("After adding row:", table.data);
table.removeRow(1);
console.log("After removing row 1:", table.data);

// Test accessing and modifying cell values by index
console.log("Cell at (0, 1):", table.getCellValue(0, 1)); // "Alice"
table.setCellValue(0, 1, "Alicia");
console.log("Updated cell at (0, 1):", table.getCellValue(0, 1)); // "Alicia"

// Test accessing and modifying cell values by column name
console.log("Cell at row 0, column 'ID':", table.getCellValueByName(0, "ID")); // 1
table.setCellValueByName(0, "Name", "Alice");
console.log("Updated cell at row 0, column 'Name':", table.getCellValueByName(0, "Name")); // "Alice"

// Test column access by name
const idColumn = table.getColumnByName("ID");
console.log("ID column:", idColumn); // Column { name: "ID", type: "number", ... }

// Test visible data
console.log("Visible data:", table.getVisibleData()); // [[1, "Alice"], [3, "Charlie"]]