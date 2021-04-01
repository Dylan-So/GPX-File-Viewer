// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {
    // On page-load AJAX Example
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/endpoint1',   //The server endpoint we are connecting to
        data: {
            data1: "Value 1",
            data2:1234.56
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            jQuery('#blah').html("On page load, received string '"+data.somethingElse+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error); 
        }
    });

    function addFileRow(gpxFile, version, creator, numWpt, numRte, numTrk) {
        var filename = encodeURIComponent(gpxFile);
        return "<tr><td><a href=uploads/" + filename +" download>" + gpxFile + "</a></td>"
        + "<td>" + version + "</td>"
        + "<td>" + creator + "</td>"
        + "<td>" + numWpt + "</td>"
        + "<td>" + numRte + "</td>"
        + "<td>" + numTrk + "</td>"
        + "</tr>";
    }

    function getFileLog(callback) {
        $("#fileLog").find("tr:gt(0)").remove()
        $.ajax({
            type:"get",
            url:"/getFiles",
            dataType:"json",
            success: callback,
            error: function(error) {
                $("#fileLog")
                .html("<tr><th>&lt;No Files&gt;</th></tr>");
                console.log(error);
            }
        });
    }

    function updateFileTable(data) {
        data.forEach(fileData => {
            var file = JSON.parse(fileData);
            $("#fileLog > tbody:last-child")
            .append($(addFileRow(file.name, file.version, file.creator, file.numWaypoints, file.numRoutes, file.numTracks)));
            $(".fileList")
            .append("<option value="+ encodeURIComponent(file.name) + ">" + file.name + "</option>");
        });
    }

    getFileLog(updateFileTable);

    function uploadFile(formData, callback) {
        $.ajax({
            url: '/upload',
            type: 'POST',
            data: formData,
            dataType: 'formData',
            contentType: false,
            processData: false,
            success: callback,
            fail: callback
        })
    }

    function checkFileExists(filename, callback) {
        $.ajax({
            url: '/fileExists',
            type: 'GET',
            data: {
                filename: filename
            },
            success: callback,
        })
    }
    // Uploading a file to server
    $("#uploadForm").submit(function(data) {
        var formData = new FormData($('#uploadForm')[0]);
        uploadFile(formData, function(data) {
            alert(data.responseText);
        })
        checkFileExists(document.getElementById("uploadFile").files[0].name, function(data) {
            if (data.exists) {
                alert("Successfully uploaded " + data.filename);
            } else {
                console.log(formData);
                alert(data.filename + " is not a valid gpx file");
            }
        }) 
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#routeBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    // Default html for gpxPanel table
    $("#gpxPanel").html("<tr><td><b>&lt;Select A File&gt;</b></td></tr>");

    // Return html code for a table row
    function getGPXRow(component, name, numPoints, length, loop) {
        return "<tr id='" + name + "'><td>" + component + "</td>"
        + "<td>"+ name +"</td>"
        + "<td>"+ numPoints +"</td>"
        + "<td>"+ length +"m</td>"
        + "<td>"+ loop +"</td>"
        + "<td><button class='expandable'>+</button></td>"
        + "</tr>"
    }

    function getOtherData(filename, type, name) {
        $.ajax({
            url:"/getOtherData",
            type:'GET',
            async:"true",
            data: {
                filename: filename,
                type: type,
                name: name
            },
            success: function(data) {
                data = JSON.parse(data);
                if (data.length == 0) {
                    $("#gpxPanel").append("<tr style='display:none;'><td></td><td></td><td></td><td></td><td></td><td></td></tr>")
                } else {
                    data.forEach(otherData => {
                        var td = document.createElement("td");
                        var id = "#" + name;
                        console.log(id);
                        document.getElementById(name).append("<tr class='info' style=''><td>" + otherData.name + "</td><td>" + otherData.value + "</td><td></td><td></td><td></td><td></td></tr>")
                    })
                }
            },
            fail: function(data) {
                console.log(data.responseText);
            },
        })
    }

    // Adds information of a GPX file to GPX Panel table
    function addGPXRow(data, filename) {
        console.log("Succuessfully received JSON data");
        var i = 1;
        $("#gpxPanel").find("tr:gt(0)").remove()
        $("#renameList").find("option:gt(0)").remove()
        var routes = JSON.parse(data[0]);
        routes.forEach(route => {
            $("#gpxPanel")
            .append(getGPXRow("Route " + i, route.name, route.numPoints, route.len, route.loop))
            getOtherData(filename, "route", route.name)
            $("#renameList").append("<option name='route' value=\"" + route.name + "\">Route "  + i + " - " + route.name + "</option>");
            i++;
        })
        var tracks = JSON.parse(data[1]);
        i = 1;
        tracks.forEach(track => {
            $("#gpxPanel")
            .append($(getGPXRow("Track " + i, track.name, track.numPoints, track.len, track.loop)))
            getOtherData(filename, "track", track.name)
            $("#renameList").append("<option name='track' value=\""+ track.name + "\">Track " + i + " - " + track.name + "</option>");
            i++;
        })
    }

    function updateGPXPanel() {
        if (!(document.getElementById("gpxList").value.includes("<select>"))) {
            $.ajax({
                url:"/docInfo",
                type:"get",
                data: {
                    filename:document.getElementById("gpxList").value.toString(),
                },
                dataType:'json',
                success: function(data) {
                    $("#gpxPanel").html("<tr><th>Component</th><th>Name</th><th>Number of Points</th><th>Length</th><th>Loop</th></tr>");
                    addGPXRow(data, document.getElementById("gpxList").value.toString());
                },
                fail: function(error) {
                    alert(error.responseText);
                }
            })
            
        } else {
            $("#gpxPanel").find("tr:gt(0)").remove()
            $("#renameList").find("option:gt(0)").remove()
        }
    }
    // When a file is selected from list, add it to gpx panel
    $("#gpxList").change(function() {
        updateGPXPanel();
    })

    // Submitting form for renaming route/track
    $('#submitButton').on('click', function () {
        var regex = new RegExp("^[a-zA-Z0-9, -]+$");
        var list =  document.getElementById("renameList");
        var newName = document.getElementById("renameBox").value;
        if (!regex.test(newName)) {
            if (!(newName === "")) {
                alert("Special Characters are not allowed!");
                return;
            }
        }
        var oldName = list.value;
        if (oldName.includes("None")) {
            oldName = "";
        }
        if (newName === oldName) {
            alert("You are renaming to the same name!");
            return;
        }
        var filename = document.getElementById("gpxList").value;
        var type = list.options[list.selectedIndex].text;
        document.getElementById("renameBox").value = "";
        if (type.includes("Track")) {
            type = "track";
        } else {
            type = "route";
        }
        
        if (!(oldName.includes("<select>") && !(filename.includes("<select>") && !(newName.includes(""))))) {
            $.ajax({
                url:'/rename',
                type:'get',
                data: {
                    filename:filename,
                    newName:newName,
                    oldName:oldName,
                    type:type,
                },
                success: function (data) {
                    console.log(data);
                    addGPXRow(data, filename);
                    console.log("Successfully renamed " + oldName + " to " + newName);
                },
                fail: function(data) {
                    alert(data);
                }
            })
        }
    }) 

    $("button").on('click', function(event) {
        event.preventDefault();
    })

    // Expands the table row for otherData of a route/track
    $('#gpxPanel').on('click', '.expandable', function(data) {
        if ($(this).closest("tr").next().is(":visible")) {
            $(this).closest("tr").next().hide();
        } else {
            $(this).closest("tr").next().show();
        }
    })
    
    
    function checkRouteExists(filename, routeName, routeExists) {
        $.ajax({
            url:'/routeExists',
            type:'get',
            data: {
                filename: filename,
                routeName: routeName,
            },
            dataType:'json',
            success: routeExists,
            fail: function(data) {
                alert(data.responseText);
            }
        })
    }

    function addRoute(filename, routeName, lat, lon, callback) {
        console.log("2 " + filename);
        console.log("4 " + routeName);
        $.ajax({
            url:'/addRoute',
            type:'get',
            data: {
                filename: filename,
                routeName: routeName,
                lat: lat,
                lon: lon,
            },
            success: callback,
            fail: function(data) {
                alert(data);
            }
        })
    }

    $('#addRoute').click(function() {
        var filename = document.getElementById("addRouteList").value.toString();
        if (filename.includes("<select>")) {
            alert("Please select a file first");
            return;
        }
        var routeName = document.getElementById("routeBox").value.toString();
        console.log(routeName);
        checkRouteExists(filename, routeName, function(result) {
            var filePath = document.getElementById("addRouteList").value.toString();
            var rteName = document.getElementById("routeBox").value.toString();
            var latBox = document.getElementById("latBox").value;
            var lonBox = document.getElementById("lonBox").value;
            if (result) {
                if (latBox === "" && lonBox === "") {
                    addRoute(filePath, rteName, -1, -1, function (data) {
                        getFileLog(updateFileTable);
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                        alert(data);
                    });
                } else if ((latBox === "" && !(lonBox === "")) || (!(latBox === "") && lonBox === "")) {
                    alert("Both latitude and longitude values are required for new routes!");
                } else {
                    addRoute(filePath, rteName, latBox, lonBox, function(data) {
                        getFileLog(updateFileTable);
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                        alert(data);
                    });
                }
            } else {
                if (latBox === "" || lonBox === "") {
                    alert("Both latitude and longitude values are required for new routes!");
                } else {
                    addRoute(filePath, rteName, latBox, lonBox, function(data) {
                        getFileLog(updateFileTable);
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                        alert(data);
                    });
                }
            }
        })
    })

    function findPaths(filename, startWpt, endWpt, callback) {
        $.ajax({
            url:'/findPaths',
            type:'GET',
            data: {
                filename: filename,
                start: startWpt,
                end: endWpt,
            },
            success: callback,
        })
    }

    $("#findPath").on('click', function() {
        var startWpt = {}
        startWpt.lat = parseFloat(document.getElementById("startLatBox").value);
        startWpt.lon = parseFloat(document.getElementById("startLonBox").value);
        console.log(startWpt);

        var endWpt = {};
        endWpt.lat = parseFloat(document.getElementById("endLatBox").value);
        endWpt.lon = parseFloat(document.getElementById("endLonBox").value);
        console.log(endWpt);
        if (Number.isNaN(startWpt.lat) || Number.isNaN(startWpt.lon)
            || Number.isNaN(endWpt.lat) || Number.isNaN(endWpt.lon)) {
            alert("Error: Both coordinates are requied for start and end points!")
            return;
        }
        $("#pathTable").html("<tr><th>Files</th></tr>")
        getFileLog(function(data) {
            data.forEach(file => {
                var fileObj = JSON.parse(file);
                findPaths(fileObj.name, startWpt, endWpt, function(message) {
                    if (message[0] !== "[]" ^ message[1] !== "[]") {
                        console.log(message);
                        if (message[0] !== "[]") {
                            var routes = JSON.parse(message[0]);
                            $('#pathTable')
                            .append("<tr><td>" + fileObj.name + "</td><td><button class='expandable'>+</button></td></tr>")
                            var i = 1;
                            routes.forEach(route => {
                                $('#pathTable').append("<tr style='display:none;'><td>Route " + i + " - " + route.name + "</td><td></td></tr>")
                            })
                        }
                        if (message[1] !== "[]") {
                            var tracks = JSON.parse(message[1]);
                            $('#pathTable')
                            .append("<tr><td>" + fileObj.name + "</td><td><button class='expandable'>+</button></td></tr>")
                            i = 1;
                            tracks.forEach(track => {
                                $('#pathTable').append("<tr style='display:none;'><td>Track " + i + " - " + track.name + "</td><td></td></tr>")
                            })
                        }
                    }
                });
            })
        });
    })

    $('#pathTable').on('click', '.expandable', function(data) {
        if ($(this).closest("tr").next().is(":visible")) {
            $(this).closest("tr").next().hide();
        } else {
            $(this).closest("tr").next().show();
        }
    })
});