/**
 * This file generates factories and classes for all the data types that are registered
 *      in the c++ code and controls the websocket connection
 * All the data types are send over the websocket by the (in the c++ code) Network::WebsocketHandler
 */

/**************************************************************************
 * Utility and constants                                                  *
 *************************************************************************/
const binaryProtocolVersion = 4;
const classStructureProtocolVersion = 3;
const websocketHandlerProtocolVersion = 1;
const websocketCheckValue = 133;

const outputFlag = {
    IncludeTypeInfo      : 1, // Defaults to not include type info
    OutputBigEndian      : 2, // Default is system endianess
    OutputLitleEndian    : 4, // Default is system endianess
    ExcludeVersioning    : 8, // Defaults to including the versioning data
    ExcludeVariableNames : 16,// Defaults to include variable names (is only output if IncludeTypeInfo=true)
    CheckVariableNames   : 32 // Defaults to not check the variable names (is only checked if IncludeTypeInfo=true)
};
const serializationTypes = {
    Class                   : 0,

    
    Uint8                   : 1,
    Uint16                  : 2,
    Uint32                  : 3,
    Uint64                  : 4,
    Uint128                 : 5,
    Int8                    : 6,
    Int16                   : 7,
    Int32                   : 8,
    Int64                   : 9,
    Int128                  : 10,
    Bool                    : 11,

    Float                   : 15,
    Double                  : 16,
    LongDouble              : 17,

    String                  : 20,

    Array                   : 30,
    Vector                  : 31,
    Deque                   : 32,
    ForwardList             : 33,
    List                    : 34,
    Set                     : 35,
    Multiset                : 36,
    Map                     : 37,
    Multimap                : 38,
    UnorderedSet            : 39,
    UnorderedMultiset       : 40,
    UnorderedMap            : 41,
    UnorderedMultimap       : 42,
    Stack                   : 43,
    Queue                   : 44,
    PriorityQueue           : 45,
    FlatSet                 : 46,
    FlatMultiset            : 47,
    FlatMap                 : 48,
    FlatMultimap            : 49,

    TimePoint               : 60,
    Duration                : 61
};
const clockTypes = {
    System              : 0,
    Steady              : 1,
    HighResolution      : 2,
    UTC                 : 3,
    TAI                 : 4,
    GPS                 : 5,
    File                : 6
};

function GetString(dataView) {
    let length = dataView.getUint16();
    let str = "";
    for(let i = 0; i < length; i++) {
        let char = String.fromCharCode(dataView.getUint8());
        str = str.concat(char);
    }
    return str;
}
/**************************************************************************
 * Byte buffer                                                            *
 *************************************************************************/
/**
 * @brief Reads sequentially from the start of the buffer, always writes at the end of the buffer
 */
class ReadWriteBuffer {
    #litleEndian = false;
    #buffer;
    #view;
    #readingPos = 0;

    constructor({ litleEndian, buffer, readingPos, maxByteLength }={}) {
        if(litleEndian != undefined) this.#litleEndian = litleEndian;
        if(buffer != undefined) {
            this.#buffer = buffer;
        } else {
            if(maxByteLength == undefined) maxByteLength = 1024;
            this.#buffer = new ArrayBuffer(0, {maxByteLength: maxByteLength});
        }
        if(readingPos != undefined) this.#readingPos = readingPos;
        this.#view = new DataView(this.#buffer);
    }

    getReadingOffset() {
        return this.#readingPos;
    }
    getBuffer() {
        return this.#buffer;
    }
    getView() {
        return this.#view;
    }

    getUint8(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getUint8(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 1;
        return result;
    }
    getUint16(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getUint16(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 2;
        return result;
    }
    getUint32(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getUint32(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 4;
        return result;
    }
    getUint64(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getUint64(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 8;
        return result;
    }

    getInt8(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getInt8(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 1;
        return result;
    }
    getInt16(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getInt16(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 2;
        return result;
    }
    getInt32(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getInt32(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 4;
        return result;
    }
    getInt64(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getInt64(this.#readingPos + offset, this.#litleEndian);
        this.#readingPos += 8;
        return result;
    }

    getFloat32(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getFloat32(this.#readingPos + offset, this.#litleEndian);// should use IEEE 754
        this.#readingPos += 4;
        return result;
    }
    getFloat64(offset) {
        if(!offset) offset = 0;
        const result = this.#view.getFloat64(this.#readingPos + offset, this.#litleEndian);// should use IEEE 754
        this.#readingPos += 8;
        return result;
    }

    setUint8(value) {
        if(this.#buffer.byteLength + 1 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 1);
        this.#view.setUint8(this.#buffer.byteLength - 1, value, this.#litleEndian);
    }
    setUint16(value) {
        if(this.#buffer.byteLength + 2 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 2);
        this.#view.setUint16(this.#buffer.byteLength - 2, value, this.#litleEndian);
    }
    setUint32(value) {
        if(this.#buffer.byteLength + 4 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 4);
        this.#view.setUint32(this.#buffer.byteLength - 4, value, this.#litleEndian);
    }
    setUint64(value) {
        if(this.#buffer.byteLength + 8 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 8);
        this.#view.setUint64(this.#buffer.byteLength - 8, value, this.#litleEndian);
    }

    setInt8(value) {
        if(this.#buffer.byteLength + 1 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 1);
        this.#view.setInt8(this.#buffer.byteLength - 1, value, this.#litleEndian);
    }
    setInt16(value) {
        if(this.#buffer.byteLength + 2 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 2);
        this.#view.setInt16(this.#buffer.byteLength - 2, value, this.#litleEndian);
    }
    setInt32(value) {
        if(this.#buffer.byteLength + 4 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 4);
        this.#view.setInt32(this.#buffer.byteLength - 4, value, this.#litleEndian);
    }
    setInt64(value) {
        if(this.#buffer.byteLength + 8 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 8);
        this.#view.setInt64(this.#buffer.byteLength - 8, value, this.#litleEndian);
    }

    setFloat32(value) {
        if(this.#buffer.byteLength + 4 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 4);
        this.#view.setFloat32(this.#buffer.byteLength - 4, value, this.#litleEndian);
    }
    setFloat64(value) {
        if(this.#buffer.byteLength + 8 > this.#buffer.maxByteLength) throw "[websocket.js] Cannot add more values then in the constructor specified maxByteLength to a ReadWriteBuffer";
        this.#buffer.resize(this.#buffer.byteLength + 8);
        this.#view.setFloat64(this.#buffer.byteLength - 8, value, this.#litleEndian);
    }

};

/**************************************************************************
 * Data handeling                                                         *
 *************************************************************************/
export class WebsocketNumberNode {
    #min = 0;
    #max = 255;
    #isInteger = true;
    #value = 0;
    constructor(min, max, isInteger) {
        this.#min = min;
        this.#max = max;
        this.#isInteger = isInteger;
    }

    Set(value) {
        if(typeof(value) != "number") throw `[websocket.js] Cannot assign something different than a number to this (tried to assign ${typeof(value)} to this)`;
        if(value < this.#min) throw `[websocket.js] Cannot assign a number lower than ${this.#min} to this`;
        if(value > this.#max) throw `[websocket.js] Cannot assign a number larger than ${this.#max} to this`;
        if(this.#isInteger && !Number.isInteger(value)) throw `[websocket.js] Cannot assign a floating point to an integer`;
        this.#value = value;
    }
    Get() {
        return this.#value;
    }
    ReadFromWebsocket(dataView, type) {
        if(type == serializationTypes.Uint8) {
            this.#value = dataView.getUint8();
            return dataView;
        } else if(type == serializationTypes.Uint16) {
            this.#value = dataView.getUint16();
            return dataView;  
        } else if(type == serializationTypes.Uint32) {
            this.#value = dataView.getUint32();
            return dataView;      
        } else if(type == serializationTypes.Uint64) {
            this.#value = dataView.getUint64();
            return dataView;      
        }

        else if(type == serializationTypes.Int8) {
            this.#value = dataView.getUint8();
            return dataView;
        } else if(type == serializationTypes.Int16) {
            this.#value = dataView.getUint16();
            return dataView; 
        } else if(type == serializationTypes.Int32) {
            this.#value = dataView.getUint32();
            return dataView;  
        } else if(type == serializationTypes.Int64) {
            this.#value = dataView.getUint64();
            return dataView;  
        }
        
        else if(type == serializationTypes.Bool) {
            this.#value = dataView.getUint8() ? true : false;
            return dataView;  
        }
        
        else if(type == serializationTypes.Float) {
            this.#value = dataView.getFloat32();// should use IEEE 754
            return dataView;
        } else if(type == serializationTypes.Double) {
            this.#value = dataView.getFloat64();// should use IEEE 754
            return dataView;
        }
        throw "[websocket.js] Unknown data type in WebsocketNumberNode";
    }
    Clone(storeID) {
        let node = new WebsocketNumberNode(this.#min, this.#max, this.#isInteger);
        node.Set(this.#value);
        return node;
    }
    Serialize(dataView, type) {
        if(type == serializationTypes.Uint8) {
            dataView.setUint8(this.#value);
        } else if(type == serializationTypes.Uint16) {
            dataView.setUint16(this.#value);
        } else if(type == serializationTypes.Uint32) {
            dataView.setUint32(this.#value);
        } else if(type == serializationTypes.Uint64) {
            dataView.setUint64(this.#value);
        }

        else if(type == serializationTypes.Int8) {
            dataView.setUint8(this.#value);
        } else if(type == serializationTypes.Int16) {
            dataView.setUint16(this.#value);
        } else if(type == serializationTypes.Int32) {
            dataView.setUint32(this.#value);
        } else if(type == serializationTypes.Int64) {
            dataView.setUint64(this.#value);
        }
        
        else if(type == serializationTypes.Bool) {
            dataView.setUint8(this.#value ? true : false);
        }
        
        else if(type == serializationTypes.Float) {
            dataView.setFloat32(this.#value);// should use IEEE 754
        } else if(type == serializationTypes.Double) {
            dataView.setFloat64(this.#value);// should use IEEE 754
        }
        else {
            throw "[websocket.js] Unknown data type in WebsocketNumberNode";
        }
        return dataView;
    }
};
export class WebsocketStringNode {
    #maxLength = 0;
    #value = "";
    constructor(maxLength) {
        this.#maxLength = maxLength;
    }

    Set(value) {
        if(typeof(value) != "string") throw `[websocket.js] Cannot assign something different than a String to this (tried to assign ${typeof(value)} to this)`;
        if(value.length > this.#maxLength) throw `[websocket.js] Cannot assign a string with a size bigger than ${this.#maxLength}`;
        this.#value = value;
    }
    Get() {
        return this.#value;
    }
    ReadFromWebsocket(dataView, type) {
        this.#value = GetString(dataView);
        return dataView;
    }
    Clone(storeID) {
        let node = new WebsocketStringNode(this.#maxLength);
        node.Set(this.#value);
        return node;
    }
    Serialize(dataView, type) {
        dataView.setUint16(this.#value.length);
        for(let i = 0; i < this.#value.length; i++) {
            dataView.setUint8(this.#value.charAt(i));
        }
        return dataView;
    }
};
export class WebsocketDataNode {
    #variableNames = [];
    #variableTypes = [];
    #proxy;
    #typeID;
    constructor(variableNames=[], variableTypes=[]) {
        this.#variableNames = variableNames;
        this.#variableTypes = variableTypes;
        // Create a proxy to monitor all assignments to this
        this.#proxy = new Proxy(this, {
            set(target, prop, value, receiver) {
                // Check if the data model has this property:
                if(!target.variableNames.includes(prop)) {
                    console.warn(`[websocket.js] Wrong assignment, property (${prop}) is not in the data model`);
                    return false;
                }

                target[prop].Set(value);
                return true;
            },
            get(target, prop, receiver) {            
                // The function you may access under the proxy
                if(prop === "IsWebsocketDataNode") return () => { return target.IsWebsocketDataNode };
                else if(prop === "GetTypeID") return () => { return target.GetTypeID() };
                else if(prop === "Serialize") return (dataView, type) => { target.Serialize(dataView, type) };
                // Check if the data model has this property:
                if(!target.variableNames.includes(prop)) {
                    console.warn(`[websocket.js] Cannot get ${prop}, property is not in the data model`);
                    return false;
                }
                const index = target.variableNames.findIndex((val) => val==prop);
                if(target.variableTypes[index] == serializationTypes.Class) {
                    return target[prop].GetProxy();
                }
                return target[prop].Get();
            },
            deleteProperty(target, prop) {
                console.warn(`[websocket.js] Cannot delete property (${prop})`);
                return false;
            }
        });
    }
    get variableNames() {
        return this.#variableNames;
    }
    get variableTypes() {
        return this.#variableTypes;
    }

    AddVariableType(name, type) {
        this.#variableNames.push(name);
        this.#variableTypes.push(type);
    }

    Set(value) {
        if(typeof value != "WebsocketDataNode") throw "[websocket.js] Cannot assign something different than a WebsocketDataNode to a WebsocketDataNode";
        // Compare the two WebsocketDataNode's
        if(value.#variableNames != this.#variableNames) throw "[websocket.js] Cannot assign a WebsocketNode to this that does not have the same amount of variableTypes";
        this.#variableNames.forEach((variableName, index) => {
            let posInValue = value.#variableNames.find(variableName);
            if(posInValue == undefined) throw "[websocket.js] When assigning to WebsocketDataNode, it must contain the same data";
            if(value.#variableTypes[posInValue] != this.#variableTypes[index]) throw "[websocket.js] Found two variableNames with different data types";
        });
        // Actually assign the data in value to this:
        this.#variableNames.forEach((variableName, index) => {
            this[variableName].Set(value[variableName]);
        })
    }

    ReadFromWebsocket(dataView, type) {
        for(let i = 0; i < this.#variableNames.length; i++) {
            dataView = this[this.#variableNames[i]].ReadFromWebsocket(dataView, this.#variableTypes[i]);
        }
        return dataView;
    }

    Clone() {
        let node = new WebsocketDataNode();
        for(let i = 0; i < this.#variableNames.length; i++) {
            node.AddVariableType(this.#variableNames[i], this.#variableTypes[i]);
            node[this.#variableNames[i]] = this[this.#variableNames[i]].Clone(undefined);
        }
        node.SetTypeID(this.#typeID);
        return node;
    }

    Serialize(dataView, type) {
        for(let i = 0; i < this.#variableNames.length; i++) {
            dataView = this[this.#variableNames[i]].Serialize(dataView, this.#variableTypes[i]);
        }
        return dataView;
    }

    SetTypeID(id) {
        this.#typeID = id;
    }
    GetTypeID() {
        return this.#typeID;
    }

    IsWebsocketDataNode() {
        return true;
    }

    GetProxy() {
        return this.#proxy;
    }
};

function CreateDataNode(dataView) {
    let {name, type, node} = CreateDataNodeInternal(dataView);
    if(type >= serializationTypes.UInt8) throw "[websocket.js] A datanode with a top node that isn't a compound node is not supported";
    return { name, node };
}
function CreateDataNodeInternal(dataView) {
    let type = dataView.getUint8();
    let name = GetString(dataView);
    let node = new WebsocketDataNode();
    if(type == serializationTypes.Class) {
        let amountMembers = dataView.getUint8();
        for(let i = 0; i < amountMembers; i++) {
            let nameNested, typeNested, nodeNested;
            ({ name: nameNested, type: typeNested, node: nodeNested } = CreateDataNodeInternal(dataView));
            node.AddVariableType(nameNested, typeNested);
            node[nameNested] = nodeNested;
        }
        return { name, type, node };
    }

    else if(type == serializationTypes.Uint8) {
        return {name, type, node: new WebsocketNumberNode(0, 2**8-1, true)};
    } else if(type == serializationTypes.Uint16) {
        return {name, type, node: new WebsocketNumberNode(0, 2**16-1, true)};        
    } else if(type == serializationTypes.Uint32) {
        return {name, type, node: new WebsocketNumberNode(0, 2**32-1, true)};        
    } else if(type == serializationTypes.Uint64) {
        return {name, type, node: new WebsocketNumberNode(0, 2**64-1, true)};        
    }

    else if(type == serializationTypes.Int8) {
        return {name, type, node: new WebsocketNumberNode(-(2**7), 2**7-1, true)};  
    } else if(type == serializationTypes.Int16) {
        return {name, type, node: new WebsocketNumberNode(-(2**15), 2**15-1, true)};  
    } else if(type == serializationTypes.Int32) {
        return {name, type, node: new WebsocketNumberNode(-(2**31), 2**31-1, true)};  
    } else if(type == serializationTypes.Int64) {
        return {name, type, node: new WebsocketNumberNode(-(2**63), 2**63-1, true)};  
    }

    else if(type == serializationTypes.Bool) {
        return {name, type, node: new WebsocketNumberNode(0, 1, true)};  
    }
    
    else if(type == serializationTypes.Float) {
        return {name, type, node: new WebsocketNumberNode(-(3.402823466*10**38), 3.402823466*10**38, false)};// following IEEE 754
    } else if(type == serializationTypes.Double) {
        return {name, type, node: new WebsocketNumberNode(-(2**1024-2**971), 2**1024-2**971, false)};// following IEEE 754
    }
    
    else if(type == serializationTypes.String) {
        return {name, type, node: new WebsocketStringNode(2**16-1)};// max length of UINT16_MAX
    }

    throw "[websocket.js] Not implemented yet";
}

/**************************************************************************
 * Websocket                                                              *
 *************************************************************************/

let server = {
    socket: undefined,
    messageTypeSize: 0,
    dataModels: new Map(),
    typeIDToName: new Map(),// Data model ID to name
    OnConnect: () => {},
    OnPacketRegister: (name, id, node) => {},
    OnMessage: (message) => {},
    OnDisconnect: () => {},
    GetPacketType: GetPacketType,// function GetPacketType(nameString) {}
    SendPacket: SendPacket// function SendPacket(<packet retrieved from GetPacketType>) {}
}
export default server;
function SetupWebsocket() {
    if(server.socket != undefined) {
        server.socket.close();
        server.socket = null;
    }
    server.socket = new WebSocket(`ws://${window.location.host}`, [`gameengine-websocket-reflection-v${websocketHandlerProtocolVersion}`]);
    server.socket.addEventListener("open", OnWebsocketConnect);
    server.socket.addEventListener("message", OnWebsocketMessage);
    server.socket.addEventListener("close", OnWebsocketDisconnect);
}
SetupWebsocket();



let tryingConnect = true;
let reconnectTimeout;
function WebsocketTryReconnect(everySecond=5) {
    tryingConnect = true;
    ShowDisconnectScreen();
    console.log(`[websocket.js] Trying to reconnect in ${everySecond} seconds`);
    reconnectTimeout = setTimeout(() => {
        SetupWebsocket();
        // OnWebsocketConnect handles the situation when the websocket connects
    }, everySecond * 1000);
}
function CloseWebsocket(event){
    server.socket.close();
}
function OnWebsocketConnect(event) {
    if(server.socket.protocol !== `gameengine-websocket-reflection-v${websocketHandlerProtocolVersion}`) {
        console.warn("[websocket.js] Websocket connected, but does not support the required protocol");
        return WebsocketTryReconnect();
    }
    console.log("[websocket.js] Websocket connected");
    clearTimeout(reconnectTimeout);
    HideDisconnectScreen();
    tryingConnect = false;

    // Reset the protocol state
    server.messageTypeSize = 0;
    server.dataModels = new Map();
    server.typeIDToName = new Map();

    server.socket.binaryType = "arraybuffer";
    window.addEventListener("beforeunload", CloseWebsocket);
    if(server.OnConnect != undefined) server.OnConnect();
}
function OnWebsocketDisconnect(event) {
    if(!tryingConnect) console.log("[websocket.js] Websocket disconnected");

    window.removeEventListener("beforeunload", CloseWebsocket);
    server.messageTypeSize = 0;
    WebsocketTryReconnect();
    if(server.OnDisconnect != undefined) server.OnDisconnect();
}
function OnWebsocketMessage(event) {
try {
    if(!typeof event.data == "ArrayBuffer") throw "[websocket.js] Received a message with the wrong binaryType";
    const view = new ReadWriteBuffer({ buffer: event.data });
    if(view.getUint8() != websocketCheckValue) throw "[websocket.js] Data check isn't 133";
    if(server.messageTypeSize == 0) {
        // This is the first message
        if(event.data.byteLength != 4) throw "[websocket.js] Received too many bytes for the first packet";
        if(view.getUint8() != binaryProtocolVersion) throw `[websocket.js] Wrong version of the binary protocol (${view.getUint8(-1, false)}!=${binaryProtocolVersion})`;
        if(view.getUint8() != classStructureProtocolVersion) throw `[websocket.js] Wrong version of the class structure protocol (${view.getUint8(-1, false)}!=${classStructureProtocolVersion})`;
        server.messageTypeSize = view.getUint8();
        if(server.messageTypeSize > 8) throw "[websocket.js] Received a messageTypeSize that is to big";
        return;
    }
    
    // Get the message type:
    let messageType = 0;
    if(server.messageTypeSize == 1) messageType = view.getUint8();
    if(server.messageTypeSize == 2) messageType = view.getUint16();
    if(server.messageTypeSize == 4) messageType = view.getUint32();
    if(server.messageTypeSize == 8) messageType = view.getUint64();

    const flags = view.getUint8();
    if(flags & outputFlag.IncludeTypeInfo) throw "[websocket.js] Don't know how to read type info";
    if(flags & outputFlag.OutputLitleEndian) throw "[websocket.js] Data over websocket must be in big endian";
    if(!(flags & outputFlag.OutputBigEndian)) throw "[websocket.js] Data over websocket must be in big endian";
    if(!(flags & outputFlag.ExcludeVersioning)) throw "[websocket.js] Data must not contain versioning";
    if(flags & outputFlag.ExcludeVariableNames) throw "[websocket.js] Data must include variable names";
    if(flags & outputFlag.CheckVariableNames) throw "[websocket.js] Data must not check variable name equality";

    // Actual message parsing:
    if(messageType == 0) {
        // It is a message registering a type
        const registeringType = view.getUint8();
        if(registeringType == 0 || registeringType == 1) throw "[websocket.js] Cannot register the type 0 and 1, these are reserved";
        if(view.getUint32() != event.data.byteLength-8) throw `[websocket.js] Packet size is incorrect (${view.getUint32(-4)} != ${event.data.byteLength}-8)`;
        if(view.getUint8() != flags) throw `[websocket.js] Message data has the wrong flags set (outer serialization has different flags from the inner data) (${view.getUint8(-1)}!=${flags})`;

        let name, node;
        ({name, node} = CreateDataNode(view));
        node.SetTypeID(registeringType);
        server.dataModels.set(name, node);
        server.typeIDToName.set(registeringType, name);
        console.log(`[websocket.js] A new packet type has been registered: ${name}`);

        server.OnPacketRegister(name, registeringType, node);
    }
    else if(messageType == 1) {
        // It is a message deregistering a type
        const deregisteringType = view.getUint8();
        const name = server.typeIDToName.get(deregisteringType);
        if(deregisteringType == 0 || deregisteringType == 1) throw "[websocket.js] Cannot register the type 0 and 1, these are reserved";
        server.dataModels.delete(name);
        server.typeIDToName.delete(deregisteringType);
        console.log(`[websocket.js] A new packet type has been deregistered: ${name}`);
    } else {
        // It is a known message type (or at least, we hope it is known)
        if(!server.dataModels.has(server.typeIDToName.get(messageType))) throw "[websocket.js] Received a message type that was not previously registered";

        let data = server.dataModels.get(server.typeIDToName.get(messageType)).Clone();
        data.ReadFromWebsocket(view);
        
        server.OnMessage(data);
    }
    if(view.getReadingOffset() != event.data.byteLength) throw `[websocket.js] Message doesn't contain the same amount of bytes as the agreed packet type`;
} catch(error) {
    console.warn(`Caught exception while processing a websocket message: '${error}'`);
    const view = new Uint8Array(event.data);
    let messageStr = "Faulty message: [";
    for(let i = 0; i < event.data.byteLength; i++) {
        if(i != 0) messageStr += ", ";
        messageStr += view[i];
    }
    messageStr += "]";
    console.log(messageStr);
}
}
/**************************************************************************
 * JS API                                                                 *
 *************************************************************************/
function GetPacketType(name) {
    const packet = server.dataModels.get(name).Clone(true);
    return packet.GetProxy();
}
function SendPacket(packet) {
    if(tryingConnect) throw "[websocket.js] Websocket is not connected, can't send a message now";
    const dataModel = new WebsocketDataNode;
    if(!packet?.IsWebsocketDataNode()) throw `[websocket.js] Can only send packets of the type WebsocketDataNode (trying to send ${typeof packet})`;
    
    let dataView = new ReadWriteBuffer();
    dataView.setUint8(websocketCheckValue);
    if(server.messageTypeSize == 1) dataView.setUint8(packet.GetTypeID());
    if(server.messageTypeSize == 2) dataView.setUint16(packet.GetTypeID());
    if(server.messageTypeSize == 4) dataView.setUint32(packet.GetTypeID());
    if(server.messageTypeSize == 8) dataView.setUint64(packet.GetTypeID());
    dataView.setUint8(10);// Binary serialization flags (exlude versioning, big endian)
    
    packet.Serialize(dataView);
    if(dataView.getView().getUint8(0, false) != websocketCheckValue) {
        const view = dataView.getView();
        let messageStr = "Faulty message: [";
        for(let i = 0; i < view.byteLength; i++) {
            if(i != 0) messageStr += ", ";
            messageStr += view.getUint8(i, false);
        }
        messageStr += "]";
        throw `[websocket.js] ReadWriteBuffer is not functioning as expected (${messageStr})`;
    }

    server.socket.send(dataView.getBuffer().slice());// Copy the whole buffer
}

/**************************************************************************
 * Disconnect screen                                                      *
 *************************************************************************/
let disconnectScreenLoaded = false;
function SetupDiconnectScreen() {
    const node = document.createElement("div");
    node.id = "websocketjs-disconnect-screen";
    document.body.appendChild(node);
    // CSS is handled in one of the css files in the css folder
    node.outerHTML = `
        <div id="websocketjs-disconnect-screen">
            <i class="fa-solid fa-cog fa-spin"></i>
            <h2>Websocket disconnected</h2>
            <p>Whoopsie daysie</p>
        </div>
    `;
    disconnectScreenLoaded = true;
}
// Timeout, to make sure we only start adding nodes when loading of the page is finished
setTimeout(SetupDiconnectScreen);

function ShowDisconnectScreen() {
    if(!disconnectScreenLoaded) return;
    document.getElementById("websocketjs-disconnect-screen").style.display = "flex";
}
function HideDisconnectScreen() {
    if(!disconnectScreenLoaded) return;
    document.getElementById("websocketjs-disconnect-screen").style.display = "none";
}