import websocket from './websocket.js';

websocket.OnMessage = (message) => {
    console.log(message);
};

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
                            if (menu.children[i].pageName == "fullscreen") {
                                if (document.fullscreenElement != null) document.exitFullscreen();
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
                    if (ev.target == this) {
                        this.classList.remove("displayed");
                    }
                };
            });
        }

        attributeChangedCallback(name, oldValue, newValue) {
            if (name == "name") this.pageName = newValue;
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
            if (name == "horizontal") this.style.gridTemplateColumns = ((100 / newValue) + "% ").repeat(newValue);
            if (name == "vertical") this.style.gridTemplateRows = ((100 / newValue) + "% ").repeat(newValue);
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
        static observedAttributes = ["name"];
        #x = 0;
        #y = 0;
        #movingPointerID;
        #radius;
        #innerRadius;
        #name = "no-name";

        constructor() {
            super();
        }

        attributeChangedCallback(name, oldValue, newValue) {
            if (name == "name") this.#name = newValue;
        }

        connectedCallback() {
            setTimeout(() => {
                this.innerHTML = `<div class="outer-circle"><div class="joycon-circle"></div></div>`;
                this.outerCircle = this.getElementsByClassName("outer-circle")[0];
                this.innerCircle = this.getElementsByClassName("joycon-circle")[0];
                this.onpointerdown = this.OnPointerDown; 
            });
        }

        OnPointerDown(event) {
            this.setPointerCapture(event.pointerId);
            this.#movingPointerID = event.pointerId;
            this.addEventListener("pointermove", this.OnPointerMove);
            this.addEventListener("pointerup", this.OnPointerUp);
        }

        OnPointerMove(event) {
            if (this.#movingPointerID != event.pointerId) return;

            this.CalculatePosition(event.clientX, event.clientY);
            this.Render();

            this.SendPosition();
        }

        OnPointerUp(event) {
            if (this.#movingPointerID != event.pointerId) return;
            this.removeEventListener("pointermove", this.OnPointerMove);
            this.removeEventListener("pointerup", this.OnPointerUp);
            this.innerCircle.style.left = "";
            this.innerCircle.style.top = "";
            this.SendPosition(0, 0);
        }

        /**
         * @brief Calculates the x,y position of the joystick
         * 
         * @param {Number} x The cursors x
         * @param {Number} y The cursors y
         */
        CalculatePosition(x, y) {
            let outerCircleBound = this.outerCircle.getBoundingClientRect();
            let innerCircleBound = this.innerCircle.getBoundingClientRect();
            if(outerCircleBound.width != outerCircleBound.height) throw "[ui.js] The outer circle of the joystick must be round";
            if(innerCircleBound.width != innerCircleBound.height) throw "[ui.js] The inner circle of the joystick must be round";
            x -= outerCircleBound.x;
            y -= outerCircleBound.y;
            // removing half of the inner circle makes sure that the innercircle stays inside the outer circle
            this.#innerRadius = 0.5 * innerCircleBound.width;
            this.#radius = 0.5 * outerCircleBound.width - this.#innerRadius;
            let middleXY = outerCircleBound.width * 0.5;
            let length = Math.sqrt((middleXY - x) ** 2 + (middleXY - y) ** 2);
            let newLength = Math.min(length, this.#radius);
            this.#x = (x - middleXY) * (newLength / length) + middleXY;
            this.#y = (y - middleXY) * (newLength / length) + middleXY;
        }

        /**
         * @brief Sets all the positions of the graphics elements
         */
        Render() {
            this.innerCircle.style.left = (this.#x - this.#innerRadius).toString() + "px";
            this.innerCircle.style.top = (this.#y - this.#innerRadius).toString() + "px";
        }

        /**
         * @brief Sends the current position over the websocket
         */
        SendPosition(x, y) {
            if(x == undefined || y == undefined) {
                // Calculate
                x = (this.#x/this.#radius - 1);
                y = (this.#y/this.#radius - 1);
            }
            let packet = websocket.GetPacketType("Engine::Network::JoystickEvent");
            packet._name = this.#name;
            packet._pos.x = x;
            packet._pos.y = y;
            websocket.SendPacket(packet);
        }
    }
);