                                      /*
@PROJECT [name: serialize]

@TASK [title: offset in references for string] [project:serialize] [assignee: per] [status:open] [created: 250828]
[summary : Adjust offsets in string references]
[description: "Adding rstring values to table are now searching if there are any similar and if found
it takes that one and doing that it saves memory, but it takes time and if time is important its
faster to use offset position to string"]

@TASK [title: save columns] [project:serialize] [assignee : per] [status:open] [created: 250827]
[summary : Save column information for table with serialization]

@TASK [title: save references] [project:serialize] [assignee : per] [status:open] [created: 250827]
[summary : Save reference data for table, this is values that have different lengths]

@TASK [title: save offset to data] [project:serialize] [assignee : per] [status:open] [created: 250827]
[summary : Save offset data for table to get direct access]
[description: When saving table data we need to save offset to data for each row, this makes it possible to read cell values directly from the disk]
*/