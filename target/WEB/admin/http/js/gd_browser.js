var gd = gd || {};

(function(oNS) {

/** ---------------------------------------------------------------------------
 * Encodes URL parameters by iterating over all search parameters and
 * automatically encoding their values.
 *
 * @param {string} sUrl - The URL string to encode.
 * @returns {string} The fully encoded URL string.
 */
oNS.EncodeUrlParams = function(sUrl) {
   try {
      // ## 1. Parse the string into a URL object
      // Note: If your input doesn't have a protocol (e.g. starts with "localhost"),
      // new URL() might fail. You can prefix it with http:// temporarily if needed.
      let oUrlObject;
      if(sUrl.startsWith('http')) {
         oUrlObject = new URL(sUrl);
      }
      else {
         // Handle inputs missing protocol (e.g. localhost:8000...)
         oUrlObject = new URL('http://' + sUrl);
      }

      // ## 2. Iterate over all search parameters
      // We modify the URL object directly. URLSearchParams automatically
      // encodes values when you set them.
      oUrlObject.searchParams.forEach((value, key) => {
         oUrlObject.searchParams.set(key, value);
      });

      // ## 3. Return the fully encoded URL string
      // If we added a fake protocol earlier, remove it for the output
      let sFinalUrl = oUrlObject.toString();
      if(!sUrl.startsWith('http')) {
         sFinalUrl = sFinalUrl.replace('http://', '');
      }

      return sFinalUrl;
   }
   catch (e_) {
      console.error("Error encoding URL:", e_);
      alert("Could not parse the URL. Ensure it looks like a valid web address.");
      return sUrl; // Return original on error
   }

/** ---------------------------------------------------------------------------
 * Converts a string to base64 format.
 *
 * @param {string} sText - The string to convert to base64.
 * @returns {string} The base64 encoded string.
 */
oNS.EncodeToBase64 = function(sText) {
   try {
      // ## Handle Unicode strings properly by encoding to UTF-8 first
      return btoa(encodeURIComponent(sText).replace(/%([0-9A-F]{2})/g,
         function toSolidBytes(match, p1) {
            return String.fromCharCode('0x' + p1);
      }));
   }
   catch (e_) {
      console.error("Error converting to base64:", e_);
      return sText; // Return original on error
   }
}

})(gd);
