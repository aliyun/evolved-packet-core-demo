/*
var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/";

MongoClient.connect(url, { useNewUrlParser: true }, function(err, db) {
    if (err) throw err;
    var dbo = db.db("vepc");
    var myobj =  [
    {
        _id : { $oid : "5ca1be2c0fbb93144dfb9ac3" },
        imsi : "460980001000201",
        pdn : [ 
        { 
            apn : "internet", 
            _id : { $oid : "5c9da7fe7490970f53b16194" }, 
            pcc_rule : [], 
            qos : { qci : 9, arp : { priority_level : 8, pre_emption_vulnerability : 0, pre_emption_capability : 0 } },
            type : 2
        }
        ],
        ambr : { downlink : { $numberLong : "1024000" }, uplink : { $numberLong : "1024000" } }, 
        subscribed_rau_tau_timer : 12, 
        network_access_mode : 2, 
        subscriber_status : 0, 
        access_restriction_data : 32, 
        security : { k : "460980001FAB283F94ADF265DC1010EF", amf : "8000", op : null, opc : "460980001E738A385CE2739DA263C09E" }, 
        __v : 0 
    }
    ];
    dbo.collection("subscribers").insertMany(myobj, function(err, res) {
        if (err) throw err;
        console.log("插入的文档数量为: " + res.insertedCount);
        db.close();
    });
});
*/

// cmd:  mongoimport --db vepc --collection "subscribers" --file ./sub.json
var fs = require("fs");

var item = {
    _id : { $oid : "5e733da4ffd69600c6ef66d2" },
    imsi : "460980001000201",
    pdn : [ 
    { 
        apn : "internet", 
        _id : { $oid : "5e1c0132e44d3700ec496677" },
        pcc_rule : [], 
        qos : { qci : 9, arp : { priority_level : 8, pre_emption_vulnerability : 0, pre_emption_capability : 0 } },
        type : 2
    }
    ],
    ambr : { downlink : { $numberLong : "1024000" }, uplink : { $numberLong : "1024000" } }, 
    subscribed_rau_tau_timer : 12, 
    network_access_mode : 2, 
    subscriber_status : 0, 
    access_restriction_data : 32, 
    security : { k : "460980001FAB283F94ADF265DC1010EF", amf : "8000", op : null, opc : "460980001E738A385CE2739DA263C09E" }, 
    __v : 0 
};


/*
var item = {
    _id : { $oid : "5ce40f5f3a00b50878f08579" },
    imsi : "440802800000002",
    pdn : [ 
    { 
        apn : "malamute.net", 
        _id : { $oid : "5ce3d5e9a5d9677a3603f282" },
        pcc_rule : [ ], 
        qos : { qci : 9, arp : { priority_level : 15, pre_emption_vulnerability : 1, pre_emption_capability : 1 } }, 
        type : 2 
    }
    ], 
    ambr : { downlink : { $numberLong : "1024000" }, uplink : { $numberLong : "1024000" } }, 
    subscribed_rau_tau_timer : 12, 
    network_access_mode : 2, 
    subscriber_status : 0, 
    access_restriction_data : 32, 
    security : { k : "24ACCE4E679F8000CAE96E24ACCECE67", amf : "8000", op : null, opc : "1B7866EAD388A55020BBEB493A3B0045" },
    __v : 0 
}
*/


var i = 0;

function hexStringAdd(hexString, num) {
    let len = hexString.length;
    let con = true;
    let pos = len-1;
    let currString = hexString;
    let n = num;
    let newString;
    var n10 = 10;

    while(con) {
        let old = parseInt(currString[pos],16);
        let new_v = old + n;
        let v_1 = new_v % 16;
        let v_2 = Math.floor(new_v / 16);

        newString = currString.substring(0,pos) + v_1.toString(16) + currString.substring(pos+1,len);
        if (v_2 > 0) {
            con = true;
            n = v_2;
        } else {
            con = false;
        }
        pos--;
        currString = newString;
    }

    return newString;
}

function intStringAdd(intString, num) {
    let len = intString.length;
    let con = true;
    let pos = len-1;
    let currString = intString;
    let n = num;
    let newString;

    while(con) {
        let old = parseInt(currString[pos],10);
        let new_v = old + n;
        let v_1 = new_v % 10;
        let v_2 = Math.floor(new_v / 10);

        newString = currString.substring(0,pos) + v_1.toString(10) + currString.substring(pos+1,len);
        if (v_2 > 0) {
            con = true;
            n = v_2;
        } else {
            con = false;
        }
        pos--;
        currString = newString;
    }
    return newString;
}

//var oid = "5ca1be2c0fbb93144dfb9ac3";
//var imsi = "460980001000201";
// op: "01020304050607080910111213141516"
// key: "24ACCE4E679F8000CAE96E24ACCECE67"
var oid = "5e733da4ffd69600c6ef66e0";
var imsi = "460980002000100";


for (i=0; i<210; i++) {
    item._id.$oid = hexStringAdd(oid, i);
    item.imsi = intStringAdd(imsi, i);
    fs.appendFile("./sub.json",  JSON.stringify(item)+'\n', (error)  => {
        if (error) return console.log("追加文件失败" + error.message);
        //console.log("追加成功");
    });
}

