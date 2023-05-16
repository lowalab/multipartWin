var config = {
    schema: "./schema/2022/20220321", // schemas directory
    cmd: "./2022-cmd.json", // navigation commands
    //xlinkBaseURL: "/srv?xlink=urn:default",
    qrcode: false, // display QR Code images on startup
    logs: true, // display history logs overlay on startup
    // HTML elements:
    qrnav: document.getElementById("qrnav"), // QR navigation
    qresp: document.getElementById("qresp"), // QR response
    qrmsg: document.getElementById("qrmsg"), // QR notify message
    nav: document.getElementById("nav"), // navigation panel
    res: document.getElementById("res"), // response panel
    msg: document.getElementById("msg"), // notify message panel
    history: document.getElementById("history") // history logs panel
};
