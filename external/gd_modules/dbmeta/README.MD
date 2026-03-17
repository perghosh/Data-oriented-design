# Metainformation about database

Manage information about database, such as tables, columns, indexes, etc. 
This module provides a way to store and retrieve metadata about the database structure and schema. 

Using metadata can help to generate tables, validate values, and perform other operations that require knowledge about the database structure.


### Files
- `gd__database.*`: Store information about the database, such as tables and columns
- `gd__statement.*`: SQL statements to manage the metadata tables, such as selecting datas, inserting data, etc.
- `gd__relations.*`: Store information about the table tree, which is a hierarchical structure that represents the relationships between tables in the database.