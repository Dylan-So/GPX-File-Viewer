// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {

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
                .html("<thead><tr><th>&lt;No Files&gt;</th></tr><thead><tbody></tbody>");
                console.log(error);
            }
        });
    }

    function updateFileTable(data) {
        data.forEach(fileData => {
            var file = JSON.parse(fileData);
            $("#fileLog > tbody:last-child")
            .append($(addFileRow(file.name, file.version, file.creator, file.numWaypoints, file.numRoutes, file.numTracks)));
        });
    }

    // Updates file panel with information about the files in the server
    // ALso updates all the file lists on the page
    getFileLog(function(data) {
        data.forEach(fileData => {
            var file = JSON.parse(fileData);
            $("#fileLog > tbody:last-child")
            .append($(addFileRow(file.name, file.version, file.creator, file.numWaypoints, file.numRoutes, file.numTracks)));
            $(".fileList")
            .append("<option value="+ encodeURIComponent(file.name) + ">" + file.name + "</option>");
        });
    });

    // Default html for gpxPanel table
    $("#gpxPanel").html("<tr><td><b>&lt;Select A File&gt;</b></td></tr>")
                  .css({"background-color" : "#009879"});
    

    // Return html code for a table row
    function getGPXRow(component, name, numPoints, length, loop) {
        return "<tr class='data'><td>" + component + "</td>"
        + "<td>"+ name +"</td>"
        + "<td>"+ numPoints +"</td>"
        + "<td>"+ length +"m</td>"
        + "<td>"+ loop +"</td>"
        + "<td><button class='expandable'>+</button></td>"
        + "</tr>"
    }

    // Sends a request for otherData JSON of a specific route/track
    // Adds the otherData information to the appropiate route/track
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
                if (data.includes("[]")) {
                    data = JSON.parse("[]");
                } else {
                    data = JSON.parse(data);
                }
                if (data.length != 0) {
                    data.forEach(otherData => {
                        otherData.value = otherData.value.replace(/(\r\n|\n|\r)/gm, "");
                        document.getElementById(name).innerHTML = ("<tr class='info' style='display:none;background-color:#f2f2f2;'><td><b>" + otherData.name + "</b></td><td>\"" + otherData.value + "\"</td><td></td><td></td><td></td><td></td></tr>")
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
            $("#gpxPanel tbody")
            .append($(getGPXRow("Route " + i, route.name, route.numPoints, route.len, route.loop)))
            .append("<tr class='info' id='"+ route.name +"' style='display:none;background-color:#ffffff;'><td></td><td><b>N/A</b></td><td></td><td></td><td></td><td></td></tr>");
            getOtherData(filename, "route", route.name, i)
            $("#renameList").append("<option name='route' value=\"" + route.name + "\">Route "  + i + " - " + route.name + "</option>");
            i++;
        })
        var tracks = JSON.parse(data[1]);
        i = 1;
        tracks.forEach(track => {
            $("#gpxPanel tbody")
            .append($(getGPXRow("Track " + i, track.name, track.numPoints, track.len, track.loop)))
            .append("<tr class='info' id='"+ track.name +"' style='display:none;background-color:#ffffff;'><td></td><td><b>N/A</b></td><td></td><td></td><td></td><td></td></tr>");
            getOtherData(filename, "track", track.name, i)
            $("#renameList").append("<option name='track' value=\""+ track.name + "\">Track " + i + " - " + track.name + "</option>");
            i++;
        })
    }

    // Updates the GPX Panel with the selected gpx file
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
                    $("#gpxPanel").html("<thead><tr><th>Component</th><th>Name</th><th>Number of Points</th><th>Length</th><th>Loop</th></tr></thead><tbody></tbody>");
                    addGPXRow(data, document.getElementById("gpxList").value.toString());
                    $('#gpxPanel tr').each(function (i, row) {
                        if ($(row).hasClass("data")) {
                            $(row).css({"background-color:" : " #f2f2f2"});
                        }
                    })
                },
                fail: function(error) {
                    console.log(error.responseText);
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
    $('#renameButton').on('click', function () {
        var regex = new RegExp("^[a-zA-Z0-9, -]+$");
        var list =  document.getElementById("renameList");
        var newName = document.getElementById("renameBox").value;
        if (list.value.includes("<select>")) {
            alert("Select a route/track first");
            return;
        }
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
                    addGPXRow(data, filename);
                    console.log("Successfully renamed " + oldName + " to " + newName);
                },
            })
        }
    }) 

    // Prevents buttons from refreshing page and losing information
    // on other tables
    $(".stopRefresh").on('click', function(event) {
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

    $('#createGPXButton').on('click', function() {
        var real = "";
        var filename = document.getElementById("fileBox").value.toString();
        var version = document.getElementById("versionBox").value;
        var creator = document.getElementById("creatorBox").value;
        if (filename === "" || version === "" || creator === "") {
            alert("All 3 fields are required to create a gpx file!");
            return;
        }
        if (!(filename.includes(".gpx"))) {
            real = filename + ".gpx";
        }
        var gpxInfo = {};
        gpxInfo.version = parseFloat(version);
        gpxInfo.creator = creator;
        $.ajax({
            url:'/createGPX',
            dataType:'json',
            data: {
                filename: real,
                gpxInfo: gpxInfo,
            },
            success: function(data) {
                getFileLog(updateFileTable);
            },
            error: function(data) {
                getFileLog(updateFileTable);
                alert(data.responseText);
            },
        })
    })
    
    // Checks if a route exists in the given file
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
                console.log(data.responseText);
            }
        })
    }

    // Adds a route to the selected file
    function addRoute(filename, routeName, lat, lon, callback) {
        $.ajax({
            url:'/addRoute',
            type:'get',
            data: {
                filename: filename,
                routeName: routeName,
                lat: lat,
                lon: lon,
            },
            dataType: 'json',
            success: callback,
            fail: function(data) {
                console.log(data.responseText)
            }
        })
    }

    // Event Listener for adding a route
    // Checks if route exists to see if it should add waypoints to a route or
    // create a new route
    $('#addRoute').click(function() {
        var filename = document.getElementById("addRouteList").value.toString();
        if (filename.includes("<select>")) {
            alert("Please select a file first");
            return;
        }
        var routeName = document.getElementById("routeBox").value.toString();
        checkRouteExists(filename, routeName, function(result) {
            var filePath = document.getElementById("addRouteList").value.toString();
            var rteName = document.getElementById("routeBox").value.toString();
            var latBox = document.getElementById("latBox").value;
            var lonBox = document.getElementById("lonBox").value;
            if (parseFloat(latBox) > 90 || parseFloat(latBox) < -90 || parseFloat(lonBox) > 180 || parseFloat(lonBox) < -180) {
                alert("Latitude cannot be less than -90 or greater than 90\nLongitude cannot be less than -180 or greater than 180");
                return;
            }
            if (result) {
                if (latBox === "" && lonBox === "") {
                    addRoute(filePath, rteName, -1, -1, function () {
                        getFileLog(updateFileTable);
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                    });
                } else if ((latBox === "" && !(lonBox === "")) || (!(latBox === "") && lonBox === "")) {
                    alert("Both latitude and longitude values are required for new routes!");
                } else {
                    addRoute(filePath, rteName, latBox, lonBox, function() { 
                        getFileLog(updateFileTable);
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                    });
                }
            } else {
                if (latBox === "" || lonBox === "") {
                    alert("Both latitude and longitude values are required for new routes!");
                } else {
                    addRoute(filePath, rteName, latBox, lonBox, function() {
                        getFileLog(updateFileTable);
                        document.getElementById("addRouteList").value = filename;
                        if (document.getElementById("addRouteList").value === document.getElementById("gpxList").value) {
                            updateGPXPanel();
                        }
                    });
                }
            }
            if (rteName !== "", latBox !== "", lonBox !== "") {
                alert(data);
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

        var endWpt = {};
        endWpt.lat = parseFloat(document.getElementById("endLatBox").value);
        endWpt.lon = parseFloat(document.getElementById("endLonBox").value);
        if (Number.isNaN(startWpt.lat) || Number.isNaN(startWpt.lon)
            || Number.isNaN(endWpt.lat) || Number.isNaN(endWpt.lon)) {
            alert("Error: Both coordinates are requied for start and end points!")
            return;
        }
        if (startWpt.lat > 90 || startWpt.lat < -90 || startWpt.lon > 180 || startWpt.lon < -180 ||
            endWpt.lat > 90 || endWpt.lat < -90 || endWpt.lon > 180 || endWpt.lon < -180) {
            alert("Latitude cannot be less than -90 or greater than 90\nLongitude cannot be less than -180 or greater than 180");
            return;
        }
        $("#pathTable").html("<thead><tr><th>Files</th><th></th></tr><thead><tbody></tbody>")
        getFileLog(function(data) {
            data.forEach(file => {
                var fileObj = JSON.parse(file);
                findPaths(fileObj.name, startWpt, endWpt, function(message) {
                    if (message[0] !== "[]" ^ message[1] !== "[]") {
                        if (message[0] !== "[]") {
                            var routes = JSON.parse(message[0]);
                            $('#pathTable tbody')
                            .append("<tr style='background-color:#f2f2f2;'><td>" + fileObj.name + "</td><td><button class='expandable'>+</button></td></tr>")
                            var i = 1;
                            routes.forEach(route => {
                                $('#pathTable tbody').append("<tr style='display:none;'><td>Route " + i + " - " + route.name + "</td><td></td></tr>")
                            })
                        }
                        if (message[1] !== "[]") {
                            var tracks = JSON.parse(message[1]);
                            $('#pathTable tbody')
                            .append("<tr style='background-color:#f2f2f2;'><td>" + fileObj.name + "</td><td><button class='expandable'>+</button></td></tr>")
                            i = 1;
                            tracks.forEach(track => {
                                $('#pathTable tbody').append("<tr class=style='display:none;'><td>Track " + i + " - " + track.name + "</td><td></td></tr>")
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

    function getNumDistance(filename, type, distance, callback) {
        $.ajax({
            url:'/getNumDistance',
            type:'GET',
            dataType:'json',
            data: {
                filename: filename,
                type: type,
                distance: distance,
            },
            success: callback,
        })
    }

    $("#distanceButton").on('click', function() {
        var routeCount = 0;
        var trackCount = 0;
        var type = document.getElementById("distanceList").value;
        var distance = document.getElementById("distanceBox").value;
        if (parseFloat(distance) < 0) {
            alert("Distance must be positive!");
            return;
        }
        if (type.includes("<select>")) {
            alert("Enter a distance!");
            return;
        }
        getFileLog(function(data) {
            data.forEach(file => {
                var fileObj = JSON.parse(file);
                getNumDistance(fileObj.name, type, distance, function(data) {
                    console.log(data);
                    routeCount += parseInt(data.route);
                    trackCount += parseInt(data.track);
                })
            })
            alert("Route Count:" + routeCount);
            alert("Track Count:" + trackCount);
        })
    })
});