class CDiv {
   constructor(options) 
   {
      const o = options;
      this.m_eParent = o.parent || document.body;
      this.m_oStyle = o.style || {"padding": "5px", "margin": "10px 0px"};
   }

   Create() {
      let eDiv;
      const eH1 = document.createElement("h1");
      eH1.textContent = "det  här är en div 000";

      eDiv = document.createElement("div");
      eDiv.classList.add("div-background");
      eDiv.appendChild(eH1.cloneNode(true));

      this.m_eParent.appendChild(eDiv);  
   }
}