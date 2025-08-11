# Tool used to clean files from selected parts

What it can do

- Remove code comments
- Remove spaces
- Remove debug code
- Join files
- Macro expansion
- Count lines
- Test simulate files affeted
- Reporting

## Count lines
Count lines in files and beeing able to count different types of lines in one session. Like counting
empty lines, comment lines and source lines. 
Results from counting lines are stored in a file or database (sqlite).

## Remove code comments
Remove code comments from files. This can be done in different ways. Also remove spaces and tabs.

## Remove selected code parts
Remove selected code parts from files. This can be done in different ways. For example remove all
console.* statements from javascript etc.


## Select language for tables

```
/rows[Age >= 30 and Country == "Sweden"]/select(Name,Age)/order_by(Age desc)/limit(5)
/rows[Score > 80 or Team == "Blue"]/select(Team,Name,Score)
/rows[City == "Göteborg"]/nth(0)/select(Name,Email)
/rows[Price >= 10.5]/slice(10:20)/select(SKU,Price)
/rows[Status != "inactive"]/count()

```