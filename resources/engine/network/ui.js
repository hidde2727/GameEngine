const socket = new WebSocket("ws://" + window.location.host);
socket.addEventListener("message", (event) => {
  console.log("Message from server ", event.data);
});
window.addEventListener("beforeunload", function(e){
  socket.close();
});

// Floating point number to a 32 bit fixed point number
function ToFixedPoint(float, sizeAfterDecimal) {
  return float * (1 << sizeAfterDecimal);
}
function Uint32ToBigEndianUint8(value) {
  const buffer = new ArrayBuffer(4);
  const view = new DataView(buffer);
  view.setUint32(0, value);
  return new Uint8Array(buffer);
}
function GetDataInBigEndian(values) {
  let size = 0;
  values.forEach(element => {
    size += parseInt(element.type.replace(/\D/g,''))/8;
  });
  const buffer = new ArrayBuffer(size);
  const view = new DataView(buffer);
  let offset = 0;
  values.forEach(element => {
    view[element.type](offset, element.value);
    offset += parseInt(element.type.replace(/\D/g,''))/8;
  });
  return new Uint8Array(buffer);
}

// Insert the css
let fontAwesome = document.createElement("link");
fontAwesome.href = "/engine/network/Font-Awesome/css/all.css";
fontAwesome.rel = "stylesheet";
document.head.appendChild(fontAwesome);

let currentStyleSheet = document.createElement("link");
currentStyleSheet.href = "/engine/network/styles/1.css";
currentStyleSheet.rel = "stylesheet";
document.head.appendChild(currentStyleSheet);

/*-----------------------------------------------------------------------------------+
| Body                                                                              |
+-----------------------------------------------------------------------------------*/
customElements.define(
  "engine-body",
  class extends HTMLElement {
    constructor() {
      super();
    }

    connectedCallback() {
      setTimeout(() => {

      });
    }
  },
);

/*-----------------------------------------------------------------------------------+
| Dropmenu                                                                           |
+-----------------------------------------------------------------------------------*/
customElements.define(
  "engine-menu",
  class extends HTMLElement {
    constructor() {
      super();
    }

    connectedCallback() {
      setTimeout(() => {
        let content = this.innerHTML;
        this.innerHTML = "";

        // Opening button
        let openButton = document.createElement("i");
        openButton.className = "fas fa-bars menu-button";
        openButton.onpointerdown = () => {
          this.displayedPage?.classList?.remove("displayed");
          this.firstPage.classList.add("displayed");
          this.displayedPage = this.firstPage;
        };
        this.appendChild(openButton);

        // Menu
        let menu = document.createElement("div");
        menu.className = "menu";
        menu.innerHTML = content;
        this.appendChild(menu);

        // Selector page
        this.firstPage = document.createElement("engine-menu-page");
        this.firstPage.classList.add("selector-page");
        this.firstPage.name = "selector";
        for (let i = 0; i < menu.children.length; i++) {
          if (menu.children[i].pageName != undefined) {
            let goToPageButton = document.createElement("div");
            goToPageButton.innerHTML = `<a>${menu.children[i].pageName}</a><i class="fas fa-arrow-right"></i>`;
            goToPageButton.className = "page-button";
            goToPageButton.onpointerdown = () => {
              if(menu.children[i].pageName == "fullscreen") {
                if (!window.screenTop && !window.screenY) document.exitFullscreen();
                else document.documentElement.requestFullscreen();
                return;
              }
              this.displayedPage?.classList?.remove("displayed");
              menu.children[i].classList.add("displayed");
              this.displayedPage = menu.children[i];
            };
            this.firstPage.appendChild(goToPageButton);
          }
        }
        menu.appendChild(this.firstPage);
      });
    }
  },
);
customElements.define(
  "engine-menu-page",
  class extends HTMLElement {
    static observedAttributes = ["name"];

    constructor() {
      super();
    }

    connectedCallback() {
      setTimeout(() => {
        this.classList.add("page");
        this.onpointerdown = (ev) => {
          if(ev.target == this) {
            this.classList.remove("displayed");
          }
        };
      });
    }
    
    attributeChangedCallback(name, oldValue, newValue) {
      if(name=="name") this.pageName = newValue;
    }
  },
);
customElements.define(
  "engine-menu-fullscreen",
  class extends HTMLElement {
    constructor() {
      super();
      this.pageName = "fullscreen";
    }
  },
);

/*-----------------------------------------------------------------------------------+
| UI layout                                                                          |
+-----------------------------------------------------------------------------------*/
customElements.define(
  "engine-grid",
  class extends HTMLElement {
    static observedAttributes = ["horizontal", "vertical"];
    constructor() {
      super();
    }
    attributeChangedCallback(name, oldValue, newValue) {
      if(name=="horizontal") this.style.gridTemplateColumns = ((100/newValue) + "% ").repeat(newValue);
      if(name=="vertical") this.style.gridTemplateRows = ((100/newValue) + "% ").repeat(newValue);
    }
  }
);
customElements.define(
  "engine-center",
  class extends HTMLElement {
    constructor() {
      super();
    }
  }
);
customElements.define(
  "engine-triangle-layout",
  class extends HTMLElement {
    constructor() {
      super();
    }
  }
);
customElements.define(
  "engine-rhombus-layout",
  class extends HTMLElement {
    constructor() {
      super();
    }
  }
);

/*-----------------------------------------------------------------------------------+
| Joystick                                                                           |
+-----------------------------------------------------------------------------------*/
customElements.define(
  "engine-joystick",
  class extends HTMLElement {
    constructor() {
      super();
    }

    connectedCallback() {
      setTimeout(() => {
        this.innerHTML = `<div class="outer-circle"><div class="joycon-circle"></div></div>`;
        this.outerCircle = this.getElementsByClassName("outer-circle")[0];
        this.innerCircle = this.getElementsByClassName("joycon-circle")[0];
        this.onpointerdown = (event) => {
          this.setPointerCapture(event.pointerId);
          function pointerMove(eventm) {
            if(event.pointerId == eventm.pointerId) {
              let viewportOffset = this.outerCircle.getBoundingClientRect();
              let innerCircleOffset = this.innerCircle.getBoundingClientRect();
              let x = eventm.clientX - viewportOffset.x;
              let y = eventm.clientY - viewportOffset.y;
              let radius = 0.5*viewportOffset.width - 0.5*innerCircleOffset.width;
              let middleXY = viewportOffset.width*0.5;
              let length = Math.sqrt((middleXY - x)**2 + (middleXY - y)**2);
              let newLength = Math.min(length, radius);
              x = (x-middleXY)*(newLength/length)+middleXY;
              y = (y-middleXY)*(newLength/length)+middleXY;
              this.innerCircle.style.left = (x-0.5*innerCircleOffset.width).toString() + "px";
              this.innerCircle.style.top  = (y-0.5*innerCircleOffset.height).toString() + "px";

              let maxSize = viewportOffset.width - innerCircleOffset.width;
              socket.send(GetDataInBigEndian([
                { value: ToFixedPoint((2*(x-0.5*innerCircleOffset.width)/maxSize-1), 31), type:"setUint32"},// Map the value from -1 till 1
                { value: ToFixedPoint((2*(y-0.5*innerCircleOffset.width)/maxSize-1), 31), type:"setUint32"} // Map the value from -1 till 1
              ]));
              console.log("x: " + (2*(x-0.5*innerCircleOffset.width)/maxSize-1) + " - y: " + (2*(y-0.5*innerCircleOffset.width)/maxSize-1) + " - maxSize: " + maxSize);
            }
          }
          this.addEventListener("pointermove", pointerMove);
          this.addEventListener("pointerup", function pointerUp(evente) {
            if(event.pointerId == evente.pointerId) {
              this.removeEventListener("pointermove", pointerMove);
              this.removeEventListener("pointerup", pointerUp);
              this.innerCircle.style.left = "";
              this.innerCircle.style.top  = "";socket.send(GetDataInBigEndian([
                { value: ToFixedPoint(0, 31), type:"setUint32"},
                { value: ToFixedPoint(0, 31), type:"setUint32"}
              ]));
            }
          });
        }
      });
    }
  }
);