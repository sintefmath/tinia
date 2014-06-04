// We load all configuration on loadtime.
window.addEventListener("load", function(event) {
    try {
        var config = "dmFyIGRpdiA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQoImRpdiIpOwp2YXIgYWJvdXQgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCJidXR0b24iKTsKYWJvdXQuaW5uZXJIVE1MPSJUaW5pYSBieSBTSU5URUYgSUNUIjsKYWJvdXQuc3R5bGUuZm9udFNpemU9IjVweDsiCmFib3V0LnN0eWxlLnJpZ2h0PSIxMHB4OyIKdmFyIGxpY2Vuc2VEaXYgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCJkaXYiKTsKbGljZW5zZURpdi5zdHlsZS5wYWRkaW5nID0gIjVweCI7CmxpY2Vuc2VEaXYuc3R5bGUucG9zaXRpb249ImFic29sdXRlIjsKbGljZW5zZURpdi5zdHlsZS5sZWZ0ID0gKGFib3V0Lm9mZnNldExlZnQgKyAxMCkgKyAicHgiOwpsaWNlbnNlRGl2LnN0eWxlLnRvcCA9IChhYm91dC5zdHlsZS50b3AgKyA0MCkrInB4IjsKbGljZW5zZURpdi5pbm5lckhUTUwgPSAiPGgzPlRoZSBUaW5pYSBQbGF0Zm9ybSBieSBTSU5URUYgSUNUPC9oMz4iCiAgICArICI8cD5TZWUgPGEgaHJlZj0naHR0cDovL3d3dy50aW5pYS5vcmcnPnd3dy50aW5pYS5vcmc8L2E+LjwvcD4iCiAgICArIjxwPkNvcHlyaWdodCAoYykgMjAxMiBTVElGVEVMU0VOIFNJTlRFRjwvcD4iIAogICAgKyAiPHA+VGhlIFRpbmlhIEZyYW1ld29yayBpcyBmcmVlIHNvZnR3YXJlOiB5b3UgY2FuIHJlZGlzdHJpYnV0ZSBpdCBhbmQvb3IgbW9kaWZ5ICIKICAgICsiaXQgdW5kZXIgdGhlIHRlcm1zIG9mIHRoZSBHTlUgQWZmZXJvIEdlbmVyYWwgUHVibGljIExpY2Vuc2UgYXMgcHVibGlzaGVkIGJ5ICIKICAgICsgInRoZSBGcmVlIFNvZnR3YXJlIEZvdW5kYXRpb24sIGVpdGhlciB2ZXJzaW9uIDMgb2YgdGhlIExpY2Vuc2UsIG9yICIKICAgICsiKGF0IHlvdXIgb3B0aW9uKSBhbnkgbGF0ZXIgdmVyc2lvbi48L3A+IgogICAgKyI8cD4gVGhlIFRpbmlhIEZyYW1ld29yayBpcyBkaXN0cmlidXRlZCBpbiB0aGUgaG9wZSB0aGF0IGl0IHdpbGwgYmUgdXNlZnVsLCAiCiAgICArImJ1dCBXSVRIT1VUIEFOWSBXQVJSQU5UWTsgd2l0aG91dCBldmVuIHRoZSBpbXBsaWVkIHdhcnJhbnR5IG9mICIKICAgICsgIk1FUkNIQU5UQUJJTElUWSBvciBGSVRORVNTIEZPUiBBIFBBUlRJQ1VMQVIgUFVSUE9TRS4gIFNlZSB0aGUgIgogICAgKyAiR05VIEFmZmVybyBHZW5lcmFsIFB1YmxpYyBMaWNlbnNlIGZvciBtb3JlIGRldGFpbHMuIDwvcD4iCiAgICArIjxwPllvdSBzaG91bGQgaGF2ZSByZWNlaXZlZCBhIGNvcHkgb2YgdGhlIEdOVSBBZmZlcm8gR2VuZXJhbCBQdWJsaWMgTGljZW5zZSAiCiAgICArImFsb25nIHdpdGggdGhlIFRpbmlhIEZyYW1ld29yay4gIElmIG5vdCwgc2VlIDxhIGhyZWY9J2h0dHA6Ly93d3cuZ251Lm9yZy9saWNlbnNlcy8nPiIKICAgICsgImh0dHA6Ly93d3cuZ251Lm9yZy9saWNlbnNlcy88L2E+LjwvcD4iOwpsaWNlbnNlRGl2LnN0eWxlLmRpc3BsYXkgPSJub25lIjsKbGljZW5zZURpdi5zdHlsZS56SW5kZXggPSAxMDA7CmxpY2Vuc2VEaXYuc3R5bGUuYm9yZGVyPSJzb2xpZCAxcHgiCmRpdi5hcHBlbmRDaGlsZChsaWNlbnNlRGl2KTsKZGl2LmFwcGVuZENoaWxkKGFib3V0KTsKbGljZW5zZURpdi5zdHlsZS5iYWNrZ3JvdW5kID0id2hpdGUiOwpsaWNlbnNlRGl2LnN0eWxlLndpZHRoID0iNTAwcHgiOwoKYWJvdXQuYWRkRXZlbnRMaXN0ZW5lcigiY2xpY2siLCBmdW5jdGlvbihldmVudCkgewogICAgaWYobGljZW5zZURpdi5zdHlsZS5kaXNwbGF5ID09Im5vbmUiKSB7CiAgICAgICAgbGljZW5zZURpdi5zdHlsZS5kaXNwbGF5PSJibG9jayI7CiAgICB9CiAgICBlbHNlIHsKICAgICAgICBsaWNlbnNlRGl2LnN0eWxlLmRpc3BsYXkgPSAibm9uZSI7CiAgICB9CiAgICBldmVudC5zdG9wUHJvcGFnYXRpb24oKTsKfSk7CmRvY3VtZW50LmJvZHkuYWRkRXZlbnRMaXN0ZW5lcigiY2xpY2siLCBmdW5jdGlvbihldmVudCkgewogICAgbGljZW5zZURpdi5zdHlsZS5kaXNwbGF5PSJub25lIjsKfSk7CmRvY3VtZW50LmJvZHkuaW5zZXJ0QmVmb3JlKGRpdiwgZG9qby5ieUlkKCJndWkiKSk7Cg==";
        var configuration = window.atob(config);




//        var div = document.createElement("div");
//        var about = document.createElement("button");
//        about.innerHTML="Tinia by SINTEF ICT";
//        about.style.fontSize="5px;"
//        about.style.right="10px;"
//        var licenseDiv = document.createElement("div");
//        licenseDiv.style.padding = "5px";
//        licenseDiv.style.position="absolute";
//        licenseDiv.style.left = (about.offsetLeft + 10) + "px";
//        licenseDiv.style.top = (about.style.top + 40)+"px";
//        licenseDiv.innerHTML = "<h3>The Tinia Platform by SINTEF ICT</h3>"
//            + "<p>See <a href='http://www.tinia.org'>www.tinia.org</a>.</p>"
//            +"<p>Copyright (c) 2012 STIFTELSEN SINTEF</p>"
//            + "<p>The Tinia Framework is free software: you can redistribute it and/or modify "
//            +"it under the terms of the GNU Affero General Public License as published by "
//            + "the Free Software Foundation, either version 3 of the License, or "
//            +"(at your option) any later version.</p>"
//            +"<p> The Tinia Framework is distributed in the hope that it will be useful, "
//            +"but WITHOUT ANY WARRANTY; without even the implied warranty of "
//            + "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
//            + "GNU Affero General Public License for more details. </p>"
//            +"<p>You should have received a copy of the GNU Affero General Public License "
//            +"along with the Tinia Framework.  If not, see <a href='http://www.gnu.org/licenses/'>"
//            + "http://www.gnu.org/licenses/</a>.</p>";
//        licenseDiv.style.display ="none";
//        licenseDiv.style.zIndex = 100;
//        licenseDiv.style.border="solid 1px"
//        div.appendChild(licenseDiv);
//        div.appendChild(about);
//        licenseDiv.style.background ="white";
//        licenseDiv.style.width ="500px";

//        about.addEventListener("click", function(event) {
//            if(licenseDiv.style.display =="none") {
//                licenseDiv.style.display="block";
//            }
//            else {
//                licenseDiv.style.display = "none";
//            }
//            event.stopPropagation();
//        });
//        document.body.addEventListener("click", function(event) {
//            licenseDiv.style.display="none";
//        });
//        document.body.insertBefore(div, dojo.byId("gui"));




        //eval(configuration);
    } catch(e) {
        console.log(e);
    }
});
