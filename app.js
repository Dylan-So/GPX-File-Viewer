'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');
const { SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS } = require('constants');
const { send } = require('process');
const { parse } = require('path');
const { start } = require('repl');
const mysql = require('mysql2/promise');
let connection = null;
let fileTable = "CREATE TABLE IF NOT EXISTS FILE (gpx_id INT AUTO_INCREMENT, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL, PRIMARY KEY (gpx_id))";
let routeTable = "CREATE TABLE IF NOT EXISTS ROUTE (route_id INT AUTO_INCREMENT, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, PRIMARY KEY (route_id), FOREIGN KEY (gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)";
let pointTable = "CREATE TABLE IF NOT EXISTS POINT (point_id INT AUTO_INCREMENT, point_index INT NOT NULL, latitude DECIMAL(11, 7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_id INT NOT NULL, PRIMARY KEY(point_id), FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)";

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

const GPXParser = ffi.Library("./libgpxparser",  {
  "createValidGPXdoc": ['void*', ['string', 'string']],
  "GPXtoJSON": ['string', ['void*']],
  "getWaypointList": ['void*', ['void*']],
  "getRouteList": ['void*', ['void*']],
  "getTrackList": ['void*', ['void*']],
  "trkListToJSON": ['string', ['void*']],
  "routeListToJSON": ['string', ['void*']],
  "validateGPXDoc": ['bool', ['void*', 'string']],
  "getTrkPts": ['int', ['void*']],
  "getRoute": ['void*', ['void*', 'string']],
  "getTrack": ['void*', ['void*', 'string']],
  "setRouteName": ['void', ['void*', 'string']],
  "setTrackName": ['void', ['void*', 'string']],
  "writeGPXdoc": ['bool', ['void*', 'string']],
  "deleteGPXdoc": ['void', ['void*']],
  "containsRoute": ['bool', ['void*', 'string']],
  "JSONtoWaypoint": ['void*', ['string']],
  "JSONtoRoute": ['void*', ['string']],
  "addRoute": ['void', ['void*', 'void*']],
  "addWaypoint": ['void', ['void*', 'void*']],
  "pathBetweenRteJSON": ['string', ['string', 'string', 'void*', 'void*']],
  "pathBetweenTrkJSON": ['string', ['string', 'string', 'void*', 'void*']],
  "gpxDataListToJSON": ['string', ['void*', 'int']],
  "getRoutesWithLength": ['string', ['void*', 'float', 'float']],
  "getTracksWithLength": ['string', ['void*', 'float', 'float']],
  "JSONtoGPX":['void*', ['string']],
  "getJSONWaypoints": ['string', ['void*']],
});

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

function removeFile(uploadFile, res) {
  var filePath = 'uploads/' + uploadFile.name;
  console.log(uploadFile.name + " not a valid gpx file");
  fs.unlinkSync(filePath, function(err) {
    if (err) {
      return res.status(400).send('Failed to remove gpx file');
    }
    res.redirect("/");
  })
}

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', async function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
  if (!uploadFile.name.includes(".gpx")) {
    return res.status(400).send('Not a gpx file.');
  }
  if(fs.existsSync('uploads/' + uploadFile.name)) {
      res.redirect("/");
  } else {
      // Use the mv() method to place the file somewhere on your server
      let validGPX;
     uploadFile.mv('uploads/' + uploadFile.name, function(err) {
        if(err) {
          return res.status(500).send(err);
        }
        var filePath = 'uploads/' + uploadFile.name;
        var gpxDoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
        validGPX = GPXParser.validateGPXDoc(gpxDoc, "gpx.xsd");
        GPXParser.deleteGPXdoc(gpxDoc)
        if (!validGPX) {
          removeFile(uploadFile, res);
        }
        res.redirect("/");
      });
  }
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {

      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 
app.get("/fileExists", function(req, res) {
  var uploadFile = req.query.filename;
  var filePath = "uploads/" + uploadFile;
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  var validGPX = GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd");
  GPXParser.deleteGPXdoc(GPXdoc);
  if (validGPX) {
    res.status(200).send({filename:req.query.filename, exists:true});
  } else {
    res.status(200).send({filename:req.query.filename, exists:false});
  }
})


app.get("/getFiles", function(req, res) {
  fs.readdir("uploads/", async (err, files) => {
    var fileNames = [];
    if (err) {
      console.log(err);
    } else {
      files.forEach(async file => {
        if (file.includes(".gpx")) {
          var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file.toString(), "gpx.xsd");
          if (GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd")) {
            var GPXdocJSON = GPXParser.GPXtoJSON(GPXdoc);
            var gpxObject = JSON.parse(GPXdocJSON);
            gpxObject.name = file.toString();
            GPXdocJSON = JSON.stringify(gpxObject);
            fileNames.push(GPXdocJSON);
            GPXParser.deleteGPXdoc(GPXdoc);
          }
        }
      })
      if (fileNames == "") {
        return res.status(404).send("error: Failed to find .gpx files in uploads/");
      } else {
        if (connection != null) {
          try {
            var [fileRow, testFields] = await connection.execute("SHOW TABLES LIKE 'FILE';")
            if (fileRow.length != 0) {
              await connection.execute("DELETE FROM FILE;");
              await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1;");
              await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1;");
              await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1;");
            }
          } catch (e) {
            console.log("Query Error: " + e)
          }
          getFiles();
        }
        res.contentType('json');
        res.send(fileNames);
      }
    }
  });
});

app.get("/getOtherData", function(req, res) {
  var filePath = "uploads/" + req.query.filename;
  var name = req.query.name.toString();
  var type = req.query.type.toString();
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  var data;
  var dataJSON;
  if (name.includes("None")) {
    name = ""
  }
  if (type.includes("route")) {
    data = GPXParser.getRoute(GPXdoc, name);
    dataJSON = GPXParser.gpxDataListToJSON(data, 0)
  } else {
    data = GPXParser.getTrack(GPXdoc, name);
    dataJSON = GPXParser.gpxDataListToJSON(data, 1);
  }
  res.send(dataJSON);
})

function getJSON(file) {
    var gpxInfo = [];
    var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file, "gpx.xsd");
    var routeJSON = GPXParser.routeListToJSON(GPXParser.getRouteList(GPXdoc));
    var trackJSON = GPXParser.trkListToJSON(GPXParser.getTrackList(GPXdoc));
    gpxInfo.push(routeJSON);
    gpxInfo.push(trackJSON);
    GPXParser.deleteGPXdoc(GPXdoc);
    return gpxInfo;
}
app.get("/docInfo", function(req, res) {
    var file = req.query.filename;
    var gpxInfo = getJSON(file);
    res.send(gpxInfo);
})

app.get("/rename", function(req, res) {
    var file = req.query.filename;
    var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file, "gpx.xsd");
    if (req.query.type.includes("route")) {
      var routeName = req.query.oldName;
      if (routeName.includes("None")) {
        routeName = "";
      }
      var route = GPXParser.getRoute(GPXdoc, req.query.oldName);
      GPXParser.setRouteName(route, req.query.newName.toString());
      if (connection != null) {
        let renameRoute = "UPDATE ROUTE SET route_name = '" + req.query.newName.toString() + "' WHERE route_name = '" + req.query.oldName + "';";
        connection.execute(renameRoute);
      }
    } else {
      var trackName = req.query.oldName.toString();
      if (trackName.includes("None")) {
        trackName = "";
      }
      var track = GPXParser.getTrack(GPXdoc, req.query.oldName.toString());
      GPXParser.setTrackName(track, req.query.newName);
    }
    var validGPX = GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd");
    if (validGPX) {
      var writeSuccess = GPXParser.writeGPXdoc(GPXdoc, "uploads/" + file);
      if (writeSuccess) {
        var gpxInfo = getJSON(file);
        GPXParser.deleteGPXdoc(GPXdoc);
        res.status(200).send(gpxInfo);
      } else {
        GPXParser.deleteGPXdoc(GPXdoc);
        res.status(460).send("Failed to write to file " + file);
      }
    } else {
      GPXParser.deleteGPXdoc(GPXdoc);
      res.status(460).send("Failed to rename, GPXdoc is invalid");
    }
})

app.get("/routeExists", function(req, res) {
  var routeName = req.query.routeName;
  var filePath = "uploads/" + req.query.filename.toString();
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  var routeExists = GPXParser.containsRoute(GPXdoc, routeName);
  GPXParser.deleteGPXdoc(GPXdoc);
  res.status(200).send(routeExists);
})

app.get("/createGPX", async function(req, res) {
  var filePath = "uploads/" + req.query.filename;
  if (fs.existsSync(filePath)) {
    res.status(409).send("File already exists on the server!")
    return;
  }
  req.query.gpxInfo.version = parseFloat(req.query.gpxInfo.version);
  var gpxString = req.query.gpxInfo;
  if (connection != null) {
    var fileExists = false;
    try {
      var [rows1, fields1] = await connection.execute("SELECT * FROM FILE;");
      for (var row of rows1) {
        if (file.toString() === row.file_name) {
          fileExists = true;
        }
      }
    } catch (e) {
      console.log("Query Error: " + e);
    }
    if (!fileExists) {
      let insertFile = "INSERT INTO FILE(file_name, ver, creator) VALUES('" + req.query.filename + "','" + gpxString.version + "','" + gpxString.creator + "');";
      await connection.execute(insertFile);
    }
  }
  gpxString = JSON.stringify(gpxString);
  var GPXdoc = GPXParser.JSONtoGPX(gpxString);
  var validGPX = GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd");
  if (!validGPX) {
    res.status(460).send("Tried to save an invalid gpx!");
  } else {
    var writeSuccess = GPXParser.writeGPXdoc(GPXdoc, filePath);
    if (writeSuccess) {
      res.status(200).send("Successfully created gpx file");
    } else {
      res.status(460).send("Failed to create to gpx file")
    }
  }
})

app.get('/addRoute', async function(req, res) {
  var filePath = "uploads/" + req.query.filename;
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  var routeName = req.query.routeName;
  // Convert JSON to Route and Waypoint
  var route;
  var containsRoute = GPXParser.containsRoute(GPXdoc, routeName);
  if (containsRoute) {
    if (routeName.includes("None")) {
      routeName = "";
    }
    route = GPXParser.getRoute(GPXdoc, routeName);
  } else {
    route = {};
    route.name = routeName;
    route = GPXParser.JSONtoRoute(JSON.stringify(route));
  }
  if (!(req.query.lat < 0 && req.query.lon < 0)) {
    var lat = parseFloat(req.query.lat);
    var lon = parseFloat(req.query.lon);
    var wpt = {};
    wpt.lat = lat;
    wpt.lon = lon;
    wpt = GPXParser.JSONtoWaypoint(JSON.stringify(wpt));
    GPXParser.addWaypoint(route, wpt);
  }
  if (!containsRoute) {
    GPXParser.addRoute(GPXdoc, route);
  }
  // Validate GPXdoc
  var validGPX = GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd");
  if (validGPX) {
    var writeSuccess = GPXParser.writeGPXdoc(GPXdoc, filePath);
    GPXParser.deleteGPXdoc(GPXdoc);
    if (writeSuccess) {
      if (connection != null) {
        try {
          var [fileRow, testFields] = await connection.execute("SHOW TABLES LIKE 'FILE';")
          if (fileRow.length != 0) {
            await connection.execute("DELETE FROM FILE;");
            await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1;");
            await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1;");
            await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1;");
          }
        } catch (e) {
          console.log("Query Error: " + e)
        }
        getFiles();
      }
      if (containsRoute) {
        res.status(200).send("Successfully added waypoint to " + routeName);
      } else {
        res.status(200).send("Successfully added route: " + routeName);
      }
    } else {
      res.status(460).send("Failed to write to gpx file");
    }
  } else {
    GPXParser.deleteGPXdoc(GPXdoc);
    res.status(460).send("Failed to add route");
  }
})

app.get("/findPaths", function(req, res) {
  var filePath = "uploads/" + req.query.filename;
  var startWpt = req.query.start;
  var endWpt = req.query.end;
  
  startWpt.lat = parseFloat(startWpt.lat);
  startWpt.lon = parseFloat(startWpt.lon);
  
  endWpt.lat = parseFloat(endWpt.lat);
  endWpt.lon = parseFloat(endWpt.lon);
  
  startWpt = GPXParser.JSONtoWaypoint(JSON.stringify(startWpt));
  endWpt = GPXParser.JSONtoWaypoint(JSON.stringify(endWpt));
  
  var routeJSON = GPXParser.pathBetweenRteJSON(filePath, "gpx.xsd", startWpt, endWpt);
  var trackJSON = GPXParser.pathBetweenTrkJSON(filePath, "gpx.xsd", startWpt, endWpt);

  var pathJSON = [];
  pathJSON.push(routeJSON);
  pathJSON.push(trackJSON);
  res.status(200).send(pathJSON);
})

app.get("/getNumDistance", function(req, res) {
  var type = req.query.type.toString();
  var distance = parseFloat(req.query.distance);
  var filePath = "uploads/" + req.query.filename;
  var routeCount = 0;
  var trackCount = 0;
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  if (type.includes("Route")) {
    routeCount += (GPXParser.getRoutesWithLength(GPXdoc, distance, 10));
  } else {
    trackCount += (GPXParser.getTracksWithLength(GPXdoc, distance, 10));
  }
  GPXParser.deleteGPXdoc(GPXdoc);
  res.status(200).send({route: routeCount, track: trackCount});
})

async function getDBStatus() {
  if (connection != null) {
    try {
      var [fileRow, fileFields] = await connection.execute("SELECT COUNT(*) AS fileCount FROM FILE;")
      var [routeRow, routeFields] = await connection.execute("SELECT COUNT(*) AS routeCount FROM ROUTE;")
      var [pointRow, pointFields] = await connection.execute("SELECT COUNT(*) AS pointCount FROM POINT;")
      var countObj = {};
      countObj.fileCount = fileRow[0].fileCount;
      countObj.routeCount = routeRow[0].routeCount;
      countObj.pointCount = pointRow[0].pointCount;
      return countObj;
    } catch (e) {
      console.log("Query Error: " + e);
    }
  } else {
    var countObj = {};
    countObj.fileCount = 0;
    countObj.routeCount = 0;
    countObj.pointCount = 0;
    return countObj;
  }
}

app.get("/login", async function(req, res) {
  var login = true;

  try {
    connection = await mysql.createConnection(req.query.dbConf);
    console.log("Logged in");
    await connection.execute(fileTable);
    await connection.execute(routeTable);
    await connection.execute(pointTable);
  } catch(e) {
    console.log("Query error: " + e);
    connection = null;
    login = false;
  } finally {
    var countObj = await getDBStatus();
    res.send({"count": countObj, "login": login});
  }
})

app.get("/checkLoginStatus", function(req, res) {
  if (connection == null) {
    res.send(false);
  } else {
    res.send(true);
  }
})

app.get("/logout", async function(req, res) {
  try {
    await connection.end();
    connection = null;
  } catch(e) {
    console.log("Error logging out: " + e);
  }
  res.send("Logged out");
})

async function getFiles() {
  if (connection != null) {
    fs.readdir("uploads/", (err, files) => {
        if (err) {
          console.log(err);
        } else {
          files.forEach(async file => {
            if (file.includes(".gpx")) {
              var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file.toString(), "gpx.xsd");
              if (GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd")) {
                var GPXdocJSON = GPXParser.GPXtoJSON(GPXdoc);
                var gpxObject = JSON.parse(GPXdocJSON);
                gpxObject.name = file.toString();
                GPXdocJSON = JSON.stringify(gpxObject);
                if (connection != null) {
                  var fileExists = false;
                  try {
                    var [rows1, fields1] = await connection.execute("SELECT * FROM FILE;");
                    for (var row of rows1) {
                      if (file.toString() === row.file_name) {
                        fileExists = true;
                      }
                    }
                  } catch (e) {
                    console.log("Query Error: " + e);
                  }
                  if (fileExists == false) {
                    try {
                      let insertFile = "INSERT INTO FILE(file_name, ver, creator) VALUES('" + file.toString() + "','" + gpxObject.version + "','" + gpxObject.creator + "');";
                      var fileId;
                      try {
                        fileId = await connection.query(insertFile);
                      } catch (e) {
                        console.log("Query Error: " + e);
                      }
                      var gpxInfo = getJSON(file);
                      var routes = JSON.parse(gpxInfo[0]);
                      fileId = fileId[0].insertId;
                      var unnamedNum = 1;
                      routes.forEach(async route => {
                        if (route.name.includes("None")) {
                          route.name = "Unnamed Route " + unnamedNum;
                          unnamedNum++;
                        }
                        let insertRoute = "INSERT INTO ROUTE(route_name, route_len, gpx_id) VALUES('" + route.name + "','" + route.len + "','" + fileId + "');";
                        if (route.name.includes("Unnamed")) {
                          route.name = "";
                        }
                        var gpxRoute = GPXParser.getRoute(GPXdoc, route.name);
                        var wptArray = GPXParser.getJSONWaypoints(gpxRoute);
                        var routeId;
                        try {
                          routeId = await connection.query(insertRoute);
                        } catch (e) {
                          console.log("Query Error: " + e);
                        }
                        var i = 0;
                        wptArray = JSON.parse(wptArray);
                        routeId = routeId[0].insertId;
                        wptArray.forEach(async wpt => {
                          var wptName = "NULL";
                          if (wpt.name !== "") {
                            wptName = wpt.name;
                          }
                          let insertWpt = "INSERT INTO POINT(point_index, latitude, longitude, point_name, route_id) VALUES(" + i + ",'" + wpt.lat + "','" + wpt.lon + "','" + wptName + "','" + routeId + "');";
                          i++;
                          try {
                            await connection.execute(insertWpt);
                          } catch (e) {
                            console.log("Query Error: " + e);
                          }
                        })
                      })
                      GPXParser.deleteGPXdoc(GPXdoc);
                    } catch (e) {
                      console.log("Query Error: " + e);
                    }
                  }
                }
              }
            }
          })
        }
      });
  }
}

app.get("/storeFiles", async function(req, res) {
  if (connection == null) {
    res.send({'success':false, 'responseText':'Please login first'});
  } else {
    try {
      try {
        var [fileRow, testFields] = await connection.execute("SHOW TABLES LIKE 'FILE';")
        if (fileRow.length != 0) {
          await connection.execute("DELETE FROM FILE;");
          await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1;");
          await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1;");
          await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1;");
        }
      } catch (e) {
        console.log("Query Error: " + e)
      }
      getFiles();
      await connection.execute("SELECT COUNT(*) FROM ROUTE");
      await connection.execute("SELECT COUNT(*) FROM POINT");
    } catch (e) {
      console.log("Query Error: " + e);
    } finally {
      var countObj = await getDBStatus();
      res.status(200).send({'count': countObj, 'success': true});
    }
  }
})

app.post("/clearData", async function(req, res) {
  if (connection != null) {
    try {
      var [fileRow, testFields] = await connection.execute("SHOW TABLES LIKE 'FILE';")
      if (fileRow.length != 0) {
        await connection.execute("DELETE FROM FILE;");
        await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 1;");
        await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 1;");
        await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 1;");
      }
      res.send("Successfully Cleared Data")
    } catch (e) {
      console.log("Query Error: " + e);
    }
  } else {
    res.send("Please login first");
  }
})

app.get("/getDBStatus", async function(req, res) {
  if (connection != null) {
    var countObj = await getDBStatus();
    res.send({'count':countObj, 'success':true});
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.get("/getAllRoutes", async function(req, res) {
  if (connection != null) {
    var [rteRows, rteFields] = await connection.execute("SELECT * FROM ROUTE;");
    var rows = [];
    for (var row of rteRows) {
      rows.push(row);
    }
    res.send({'success':true, 'routes':rows});
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.get("/getRoutesFromFile", async function(req, res) {
  if (connection != null) {
    var rows = [];
    try {
      var getGPXId = "SELECT gpx_id FROM FILE WHERE file_name = '" + req.query.filename + "';";
      var [gpxIdRows, gpxIdFields] = await connection.execute(getGPXId);
      var gpxId = gpxIdRows[0].gpx_id;
      var getRoutes = "SELECT * FROM ROUTE WHERE gpx_id = " + gpxId + ";";
      var [rteRows, rteFields] = await connection.execute(getRoutes);
      for (var row of rteRows) {
        rows.push(row);
      }
    } catch (e) {
      console.log("Query Error " + e);
    }
    res.send({'success':true, 'routes':rows});
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.get("/getPointsFromRoute", async function(req, res) {
  if (connection != null) {
    var rows = [];
    try {
      var getRteId = "SELECT route_id FROM ROUTE WHERE route_name = '" + req.query.routeName + "';";
      var [rteIdRows, rteIdFields] = await connection.execute(getRteId);
      var rteId = rteIdRows[0].route_id;
      console.log(rteIdRows);
      var getRoutes = "SELECT * FROM POINT WHERE route_id = " + rteId + ";";
      var [wptRows, wptFields] = await connection.execute(getRoutes);
      for (var row of wptRows) {
        rows.push(row);
      }
      res.send({'success':true, 'points':rows});
    } catch (e) {
      console.log("Query Error " + e);
    }
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.get("/getPointsFromFile", async function(req, res) {
  if (connection != null) {
    var points = [];
    try {
      var getGPXId = "SELECT gpx_id FROM FILE WHERE file_name = '" + req.query.filename + "';";
      var [gpxIdRows, gpxIdFields] = await connection.execute(getGPXId);
      var gpxId = gpxIdRows[0].gpx_id;
      var getRoutes = "SELECT route_id FROM ROUTE WHERE gpx_id = " + gpxId + ";";
      var [rteRows, rteFields] = await connection.execute(getRoutes);
      for (var row of rteRows) {
        var getPoints = "SELECT * FROM POINT WHERE route_id = " + row.route_id + ";";
        var getRteName = "SELECT route_name FROM ROUTE WHERE route_id = " + row.route_id + ";";
        var getRteLength = "SELECT route_len FROM ROUTE WHERE route_id = " + row.route_id + ";";
        var [rteLenRow, rteLenFields] = await connection.execute(getRteLength);
        var rteLen = rteLenRow[0].route_len;
        var [rteNameRow, rteNameFields] = await connection.execute(getRteName);
        var rteName = rteNameRow[0].route_name;
        var [pointRows, pointFields] = await connection.execute(getPoints);
        for (var pointRow of pointRows) {
          pointRow.route_name = rteName;
          pointRow.route_len = rteLen;
          points.push(pointRow);
        }
      }
    } catch (e) {
      console.log("Query Error " + e);
    }
    res.send({'success':true, 'points':points});
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.get("/getRoutesByLength", async function(req, res) {
  if (connection != null) {
    var rows = [];
    try {
      var getGPXId = "SELECT gpx_id FROM FILE WHERE file_name = '" + req.query.filename + "';";
      var [gpxIdRows, gpxIdFields] = await connection.execute(getGPXId);
      var gpxId = gpxIdRows[0].gpx_id;
      var getRoutes;
      if (req.query.type.includes("Shortest")) {
        getRoutes = "SELECT * FROM ROUTE WHERE gpx_id = " + gpxId + " ORDER BY route_len ASC limit " + req.query.numRoutes + ";"
      } else {
        getRoutes = "SELECT * FROM ROUTE WHERE gpx_id = " + gpxId + " ORDER BY route_len DESC limit " + req.query.numRoutes + ";";
      }
      var [rteRows, rteFields] = await connection.execute(getRoutes);
      for (var row of rteRows) {
        rows.push(row);
      }
    } catch (e) {
      console.log("Query Error " + e);
    }
    res.send({'success':true, 'routes':rows});
  } else {
    res.send({'success':false, 'responseText':'Please login first'});
  }
})

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);