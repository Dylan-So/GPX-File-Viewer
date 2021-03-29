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

    $.ajax({
        type:"get",
        url:"/getFiles",
        dataType:"json",
        success: function(data) {
            data.forEach(fileData => {
                var file = JSON.parse(fileData);
                $("#fileLog > tbody:last-child")
                .append($(addFileRow(file.name, file.version, file.creator, file.numWaypoints, file.numRoutes, file.numTracks)));
                $("#gpxList")
                .append("<option value="+ encodeURIComponent(file.name) + ">" + file.name + "</option>");
                console.log("GPX file found: " + file.name);
            });
        },
        error: function(error) {
            $("#fileLog")
            .html("<tr><th>No Files</th></tr>");
            console.log(error);
        }
    });

    $("#uploadForm").submit(function(data) {
        var formData = new FormData($('#uploadForm')[0]);
        $.ajax({
            url: '/upload',
            type: 'POST',
            data: formData,
            contentType: false,
            processData: false,
            success: function(message) {
                alert(message);
                console.log("Successfully added " + $('#uploadForm')[0].files.name);
            },
            error: function(error) {
                alert(error.responseText);
            }
        })
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    function addGPXRow(component, name, numPoints, length, loop) {
        return "<tr><td>" + component + "</td>"
        + "<td>"+ name +"</td>"
        + "<td>"+ numPoints +"</td>"
        + "<td>"+ length +"m</td>"
        + "<td>"+ loop +"</td>"
        + "<td><button class='expandable'>+</button></td>"
        + "</tr>"
    }


    // When a file is selected from list, add it to gpx panel
    $("#gpxList").change(function(selected) {
        console.log(document.getElementById("gpxList").value);
        if (!(document.getElementById("gpxList").value.includes("<select a file>"))) {
            $.ajax({
                url:"/docInfo",
                type:"get",
                data: {
                    filename:document.getElementById("gpxList").value.toString(),
                },
                dataType:'json',
                success: function(data) {
                    var i = 1;
                    $("#gpxPanel").find("tr:gt(0)").remove()
                    var routes = JSON.parse(data[0]);
                    routes.forEach(route => {
                        if ($("#gpxPanel").length > 2) {
                            $("#gpxPanel")
                            .append($(addGPXRow("Route " + i, route.name, route.numPoints, route.len, route.loop)))
                            .append("<tr style='display:none;'><td>Name</td><td>Route Name</td></tr>")
                        } else {
                            $("#gpxPanel")
                            .append($(addGPXRow("Route " + i, route.name, route.numPoints, route.len, route.loop)))
                            .append("<tr style='display:none;'><td>Name</td><td>Route Name</td></tr>")
                        }
                        i++;
                    })
                    var tracks = JSON.parse(data[1]);
                    console.log(tracks);
                    tracks.forEach(track => {
                        if ($("#gpxPanel").length > 2) {
                            $("#gpxPanel")
                            .append($(addGPXRow("Track " + i, track.name, track.numPoints, track.len, track.loop)))
                            .append("<tr style='display:none;'><td>Name</td><td>Route Name</td></tr>")
                        } else {
                            $("#gpxPanel")
                            .append($(addGPXRow("Track " + i, track.name, track.numPoints, track.len, track.loop)))
                            .append("<tr style='display:none;'><td>Name</td><td>Track Name</td></tr>")
                        }
                        i++;
                    })
                },
                fail: function(error) {
                    console.log(error);
                }
            })
        }
    })

    $('#gpxPanel').on('click', '.expandable', function(data) {
        console.log("button pressed");
        console.log($(this).closest("tr").html());
        if ($(this).closest("tr").next().is(":visible")) {
            $(this).closest("tr").next().hide();
        } else {
            $(this).closest("tr").next().show();
        }
    })
});