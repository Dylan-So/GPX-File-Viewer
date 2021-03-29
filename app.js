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

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

const GPXParser = ffi.Library("./parser/bin/libgpxparser",  {
  "createValidGPXdoc": ['void*', ['string', 'string']],
  "GPXtoJSON": ['string', ['void*']],
  "getWaypointList": ['void*', ['void*']],
  "getRouteList": ['void*', ['void*']],
  "getTrackList": ['void*', ['void*']],
  "trackListToJSON": ['string', ['void*']],
  "routeListToJSON": ['string', ['void*']],
  "validateGPXDoc": ['bool', ['void*', 'string']],
  "getTrkPts": ['int', ['void*']],
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
      uploadFile.mv('uploads/' + uploadFile.name, function(err) {
        if(err) {
          return res.status(500).send(err);
        }
        res.status(200);
        res.send(uploadFile.name + " successfully uploaded");
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
  if (req.xhr) {
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
} else {
  res.redirect("/");
}
});

app.get("/docInfo", function(req, res) {
  if (req.xhr) {
    var file = req.query.filename;
    var gpxInfo = [];
    var GPXdoc = GPXParser.createValidGPXdoc("uploads/" + file, "gpx.xsd");
    var routeJSON = GPXParser.routeListToJSON(GPXParser.getRouteList(GPXdoc));
    var trackJSON = GPXParser.trackListToJSON(GPXParser.getTrackList(GPXdoc));
    gpxInfo.push(routeJSON);
    gpxInfo.push(trackJSON);
    res.send(gpxInfo);
  } else {
    res.redirect("/")
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