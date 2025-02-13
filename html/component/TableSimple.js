class CTableSimple 
{
   static m_sName = "uitable";

   constructor(options) 
   {
      const o = options;

      this.m_eParent = o.parent || document.body;
      this.m_iRow = o.row || 5;
      this.m_iColumn = o.column || 5;
      this.m_eTable = null;
   }

   Create() 
   {
      const iColumns = this.m_iColumn;
      const iRows = this.m_iRow;

      const eTable = document.createElement("table");

      for (let i = 0; i <= iRows; i++)
      {
         const eRow = document.createElement("tr");
         /*eRow.bgColor = "#FF0000"

         if (i == 0)
         {
            eRow.bgColor = "#333333"
         }

         if (i % 2) {
            //eRow.bgColor = RandomRGBColor();
            eRow.bgColor = "#FF6666"
         }       */

         for (let j = 0; j <= iColumns; j++)
         {
            if (i == 0)
            {
               const eTableHeader = document.createElement("th");
               eTableHeader.textContent = "Det här är en tableheader";
               eTableHeader.style.color = "gray";

               eRow.appendChild(eTableHeader.cloneNode(true));
            }
            else
            {
               const eColumn = document.createElement("td");
               eColumn.textContent = "Dethär är en column";

               eRow.appendChild(eColumn.cloneNode(true));
            }
         }
         eTable.appendChild(eRow.cloneNode(true));
      }

      this.m_eTable = eTable;
      this.m_eParent.appendChild(eTable);  
   }

   Show() 
   {
      if(this.m_eTable) 
      {
         this.m_eTable.style.display = "block";
      }
   }

   Hide() 
   {
      if(this.m_eTable) 
      {
         this.m_eTable.style.display = "none";
      }
   }
}