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

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

const GPXParser = ffi.Library("./parser/bin/libgpxparser",  {
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
     uploadFile.mv('uploads/' + uploadFile.name, function(err, result) {
        if(err) {
          return res.status(500).send(err);
        }
        var filePath = 'uploads/' + uploadFile.name;
        var gpxDoc = GPXParser.createValidGPXdoc(filePath, "gpx.xsd");
        validGPX = GPXParser.validateGPXDoc(gpxDoc, "gpx.xsd");
        GPXParser.deleteGPXdoc(gpxDoc)
        if (!validGPX) {
          removeFile(uploadFile, res);
          res.status(460).send(uploadFile.name + " is not a valid gpx file!");
        } else {
          res.status(200).send("Successfully added " + uploadFile.name);
        }
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
    console.log(file);
    var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file, "gpx.xsd");
    console.log(req.query.oldName);
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
      console.log(writeSuccess);
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
  console.log(routeName);
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
  console.log("1 " + filePath);
  console.log("2 " + routeName);
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
      res.status(200).send(true);
    } else {
      res.status(460).send("Failed to write to gpx file");
    }
  } else {
    GPXParser.deleteGPXdoc(GPXdoc);
    res.status(460).send("Failed to add route");
  }
})

//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.data1 + " " + req.query.data2;
  res.send(
    {
      somethingElse: retStr
    }
  );
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);