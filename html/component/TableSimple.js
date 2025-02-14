class CTableSimple 
{
   static m_sWidgetName_s = "uitable";

   constructor(options) 
   {
      const o = options;

      // unique id for the component
      this.m_sId = CTableSimple.m_sWidgetName_s + `${Date.now()}-${Math.floor(Math.random() * 1000000)}`;

      this.m_oStyle = o.style || { "background-color": "#f0f0f0", "border-collapse": "collapse", "color": "#808080", "margin-bottom": "10px", "padding": "10px" }; // set style for the component
      this.m_oStyleTD = { "border": "1px dashed #808080", "padding": "5px", "text-align": "center" }; // set style for td elements



      this.m_eParent = o.parent || null;                                      // set parent element for table
      this.m_iRow = o.row || 5;
      this.m_iColumn = o.column || 5;
      this.m_eTable = null;
   }

   get id() { return this.m_sId; }
   get parent() { return this.m_eParent; }
   get component() { return this.m_eComponent; }


   Create( parent_ ) 
   {
      let eParent; // HTML element to hold the component
      if(typeof parent_ === "string") 
      {
         eParent = document.getElementById(parent_);
         if(eParent === null) { eParent = document.querySelector(parent_); }
      }
      else if( parent_ instanceof HTMLElement )
      { 
         eParent = parent_; 
      }
      else eParent = this.m_eParent;

      const eComponent = document.createElement('div');
      Object.assign(eComponent.dataset, { section: "component", id: this.m_sId, widget: CTableSimple.m_sWidgetName_s }); // set data- items

      const iColumns = this.m_iColumn;
      const iRows = this.m_iRow;

      const eTable = document.createElement("table");
      if( typeof this.m_oStyle === "object" ) Object.assign(eTable.style, this.m_oStyle); // set style

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
               eTableHeader.textContent = "header";
               eTableHeader.style.color = "gray";

               eRow.appendChild(eTableHeader);
            }
            else
            {
               const eTD = document.createElement("td");
               eTD.textContent = "Det här är en column";
               if( this.m_oStyleTD ) Object.assign(eTD.style, this.m_oStyleTD);

               eRow.appendChild(eTD);
            }
         }
         eTable.appendChild(eRow.cloneNode(true));
      }

      eComponent.appendChild(eTable);                                          // append table to component
      eParent.appendChild(eComponent);                                         // append component to parent, this will render the component

      this.m_eComponent = eComponent;
      this.m_eParent = eParent;
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

   SetCellValue(iRow, iColumn, sValue) 
   {
      const eRow = this.m_eTable.rows[iRow];
      if(eRow) 
      {
         const eCell = eRow.cells[iColumn];
         if(eCell) 
         {
            eCell.textContent = sValue;
         }
      }
   }
}