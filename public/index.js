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

    function addFileRow(gpxFile) {
        var filename = encodeURIComponent(gpxFile);
        return "<tr><td><a href=uploads/" + filename +" download>" + gpxFile + "</a></td>"
        + "<td>Version</td>"
        + "<td>Creator</td>"
        + "<td>Waypoints</td>"
        + "<td>Routes</td>"
        + "<td>Tracks</td>"
        + "</tr>";
    }

    $.ajax({
        type:"get",
        url:"/getFiles",
        dataType:"json",
        success: function(data) {
            data.forEach(file => {
                $("#fileLog > tbody:last-child")
                .append($(addFileRow(file)));
                $("#gpxList")
                .append("<option value="+ encodeURIComponent(file) + ">" + file + "</option>");
                console.log("GPX file found: " + file);
            });
        },
        error: function(error) {
            $("#fileLog")
            .html("<tr><th>No Files</th></tr>");
            console.log(error.responseText);
        }
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

    function addGPXRow(gpxFile, name, numPoints, length, loop) {
        var filename = encodeURIComponent(gpxFile);
        return "<tr><td>Route</td>"
        + "<td>"+ name +"</td>"
        + "<td>"+ numPoints +"</td>"
        + "<td>"+ length +"m</td>"
        + "<td>"+ loop.toString().toUpperCase() +"</td>"
        + "<td class=\"notPanel\"><button>+</button></td>"
        + "</tr>";
    }

    $("#gpxList").change(function(selected) {
        $("#gpxPanel > tbody:last-child")
        .append($(addGPXRow(this.value, "Random Name", 0, 100, true)));
    if ($("#hiddenRow").is(":visible")) {
        $("#hiddenRow").hide();
    } else {
        $("#hiddenRow").show();
    }
    console.log(decodeURIComponent(this.value));
    })
    $(".button").click(function(button) {
        if ($("#hiddenRow").is(":visible")) {
            $("#hiddenRow").hide();
        } else {
            $("#hiddenRow").show();
        }
    })
});