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
  "gpxDataListToJSON": ['string', ['void*', 'int']]
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
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
  if (!uploadFile.name.includes(".gpx")) {
    return res.status(400).send('Not a gpx file.');
  }
  if(fs.existsSync('uploads/' + uploadFile.name)) {
    res.status(460).send('File: ' + uploadFile.name + " already exists on the server!");
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
      });
      res.redirect("/");
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
  fs.readdir("uploads/", (err, files) => {
    var fileNames = [];
    if (err) {
      console.log(err);
    } else {
      files.forEach(file => {
        if (file.includes(".gpx")) {
          var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file.toString(), "gpx.xsd");
          if (GPXParser.validateGPXDoc(GPXdoc, "gpx.xsd")) {
            var GPXdocJSON = GPXParser.GPXtoJSON(GPXdoc);
            var gpxObject = JSON.parse(GPXdocJSON);
            gpxObject.name = file.toString();
            GPXdocJSON = JSON.stringify(gpxObject);
            fileNames.push(GPXdocJSON);
          }
          GPXParser.deleteGPXdoc(GPXdoc);
        }
      })
      if (fileNames == "") {
        return res.status(404).send("error: Failed to find .gpx files in uploads/");
      } else {
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
      var route = GPXParser.getRoute(GPXdoc, req.query.oldName);
      GPXParser.setRouteName(route, req.query.newName.toString());
    } else {
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

app.get('/addRoute', function(req, res) {
  var filePath = "uploads/" + req.query.filename;
  var GPXdoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
  var routeName = req.query.routeName;
  // Convert JSON to Route and Waypoint
  var route;
  var containsRoute = GPXParser.containsRoute(GPXdoc, routeName);
  if (containsRoute) {
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

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);