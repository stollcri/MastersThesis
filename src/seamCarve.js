/*
 * http://inst.eecs.berkeley.edu/~cs199-ap/
 */

$(document).ready(function(){

	var canvas = document.getElementById("myCanvas");
    var horizcanvas = document.getElementById("myHorizCanvas");
    var gradientcanvas = document.getElementById("gradientCanvas");
	var context = canvas.getContext("2d");
    var horizcontext = horizcanvas.getContext("2d");
    var gradientcontext = gradientcanvas.getContext("2d");
    var img = new Image;
    var horizimg = new Image;
    var gradientimg = new Image;
    gradientimg.src = "images.jpeg";
    horizimg.src = "balloon.jpg";
    img.src = "images.jpeg";
	var mouseDown = false;
	var hasText = true;
	var imgWidth = -1;
	var imgHeight = -1;
	var lod = 0; //level of detail
    var horiz_lod = 0; //level of detail horizontal
	var seam_highlight = false;

	//array of seams to change lod
	var cut_seams = [];
    var horiz_cut_seams = [];

	var clearCanvas = function () {
		if (hasText) {
			context.clearRect(0, 0, canvas.width, canvas.height);
			hasText = false;
		}
	};

	// Adding instructions
	context.fillText("Drop an image onto the canvas", 240, 200);
	context.fillText("Click a spot to set as brush color", 240, 220);

	// Image for loading
	img.addEventListener("load", function () {
		clearCanvas();
		canvas.height = img.height;
		canvas.width = img.width*1.25;
		imgHeight = img.height;
		imgWidth = img.width;
        $("#width-slider").slider({max: img.width*1.25, value: img.width});
        $("#width-slider").slider('setValue', img.width);
		context.drawImage(img, 0, 0);

		var imgData = context.getImageData(0,0,imgWidth,imgHeight);
		cut_seams = Filters.get_paths(imgData);
		lod = 0;
        gradientcontext.clearRect(0, 0, gradientcanvas.width, gradientcanvas.height);
		gradientcanvas.height = gradientimg.height;
		gradientcanvas.width = gradientimg.width;
		gradientImgHeight = gradientimg.height;
		gradientImgWidth = gradientimg.width;
		gradientcontext.drawImage(gradientimg, 0, 0);
        var imgData = gradientcontext.getImageData(0,0,gradientImgWidth,gradientImgHeight);
        var output = gradientcontext.createImageData(gradientImgWidth, gradientImgHeight);
        var pixels = imgData.data;
        var pixel_data = [];
        for (var i=0; i < pixels.length; i++) {
            if (i%4 === 3) continue;
            pixel_data.push(pixels[i]);
        }
        var energies = Filters.energy1(pixel_data,gradientImgWidth,gradientImgHeight);
        for (var i=0; i < energies.length; i++) {
            output.data[i*4] = energies[i];
            output.data[i*4+1] = energies[i];
            output.data[i*4+2] = energies[i];
            output.data[i*4+3] = 255;
        }
        gradientcontext.putImageData(output,0,0);
	}, false);

    horizimg.addEventListener("load", function () {
        horizcontext.clearRect(0, 0, horizcanvas.width, horizcanvas.height);
		horizcanvas.height = horizimg.height;
		horizcanvas.width = horizimg.width;
		horizImgHeight = horizimg.height;
		horizImgWidth = horizimg.width;
        $("#height-slider").slider({max: horizimg.height, value: horizimg.height});
        $("#height-slider").slider();
		horizcontext.drawImage(horizimg, 0, 0);

		var horizImgData = horizcontext.getImageData(0,0,horizImgWidth,horizImgHeight);
		horiz_lod = 0;
        horiz_cut_seams = Filters.get_horiz_paths(horizImgData); // uncomment to do horizontal

    }, false);

    gradientimg.addEventListener("load", function () {
    }, false);

	// To enable drag and drop
	canvas.addEventListener("dragover", function (evt) {
		evt.preventDefault();
	}, false);

    horizcanvas.addEventListener("dragover", function (e) {
        e.preventDefault();
    }, false);

    horizcanvas.addEventListener('drop', function (e) {
		var files = e.dataTransfer.files;
		if (files.length > 0) {
			var file = files[0];
			if (typeof FileReader !== "undefined" && file.type.indexOf("image") != -1) {
				var reader = new FileReader();
				// Note: addEventListener doesn't work in Google Chrome for this event
				reader.onload = function (e) {
					horizimg.src = e.target.result;
				};
				reader.readAsDataURL(file);
			}
		}
		e.preventDefault();
    }, false);

	// Handle dropped image file - only Firefox and Google Chrome
	canvas.addEventListener("drop", function (evt) {
		var files = evt.dataTransfer.files;
		if (files.length > 0) {
			var file = files[0];
			if (typeof FileReader !== "undefined" && file.type.indexOf("image") != -1) {
				var reader = new FileReader();
				// Note: addEventListener doesn't work in Google Chrome for this event
				reader.onload = function (evt) {
					img.src = evt.target.result;
					gradientimg.src = evt.target.result;
				};
				reader.readAsDataURL(file);
			}
		}
		evt.preventDefault();
	}, false);

    gradientcanvas.addEventListener('drop', function (e) {
		var files = e.dataTransfer.files;
		if (files.length > 0) {
			var file = files[0];
			if (typeof FileReader !== "undefined" && file.type.indexOf("image") != -1) {
				var reader = new FileReader();
				// Note: addEventListener doesn't work in Google Chrome for this event
				reader.onload = function (e) {
					gradientimg.src = e.target.result;
				};
				reader.readAsDataURL(file);
			}
		}
		e.preventDefault();
    }, false);

	// Detect mousedown
	canvas.addEventListener("mousedown", function (evt) {
		mouseDown = true;
	}, false);

	// Detect mouseup
	canvas.addEventListener("mouseup", function (evt) {
		mouseDown = false;
	}, false);

	// Draw, if mouse button is pressed
	canvas.addEventListener("mousemove", function (evt) {
		if (mouseDown) {
			//resizeImage(1);
		}
	}, false);

    var array_to_image = function(energies, width, height) {
        var output = context.createImageData(width, height);
        for (var i=0; i < energies.length; i++) {
            output.data[i*4] = energies[i];
            output.data[i*4+1] = energies[i];
            output.data[i*4+2] = energies[i];
            output.data[i*4+3] = 255;
        }
        context.putImageData(output,0,0);
        return output;
    };

    $("#width-slider").on('slide', function(e) {
        var amount = e.value-imgWidth;
        if (amount < 0) down_lod(Math.abs(amount));
        else up_lod(amount);
    });

    $("#height-slider").on('slide', function(e) {
        var amount = e.value-horizImgHeight;
        if (amount < 0) down_horiz_lod(Math.abs(amount));
        else up_horiz_lod(amount);
    });

    $(document).keydown(function(e){
        // keypad left
        if (e.keyCode == 37) {
			down_lod(4);
            $("#width-slider").slider('setValue',imgWidth);
            e.preventDefault();
            return false;
        // keypad right
        } else if (e.keyCode == 39) {
			up_lod(4);
            $("#width-slider").slider('setValue',imgWidth);
            e.preventDefault();
            return false;
        } else if (e.keyCode == 38) {
            down_horiz_lod(1);
            $("#height-slider").slider('setValue',horizImgHeight);
            e.preventDefault();
            return false;
        } else if (e.keyCode == 40) {
            up_horiz_lod(1);
            $("#height-slider").slider('setValue',horizImgHeight);
            e.preventDefault();
            return false;
        }
    });

    $("#energy1").click(function() {
        var imgData = context.getImageData(0,0,imgWidth,imgHeight);
        context.clearRect(0, 0, canvas.width, canvas.height);
        var energies = Filters.energy1(imgData);
        var newimg = array_to_image(energies, imgWidth, imgHeight);
        context.putImageData(newimg, 0, 0);
    });

	$("#highlight").click(function(){
		seam_highlight = !seam_highlight;
	});


	var remove_row = function(path){
		var imgData = context.getImageData(0, 0, imgWidth, imgHeight); // single dimension array of RGBA
        imgWidth -= 1;

        var newImg = context.createImageData(imgWidth, imgHeight);

		var path_index = 0;
		var new_index = 0;
		var dirty_x = 0;

		for (var i=0; i < imgData.data.length/4; i+=1){
			if (path[path_index].getIndex(imgWidth+1,path_index) === i){
				dirty_x = Math.min(dirty_x, path[path_index].index);
				path_index = Math.min(path_index+1,path.length-1);
				continue;
			}
			newImg.data[4*new_index] = imgData.data[4*i];
			newImg.data[4*new_index+1] = imgData.data[4*i+1];
			newImg.data[4*new_index+2] = imgData.data[4*i+2];
			newImg.data[4*new_index+3] = imgData.data[4*i+3];
			new_index++;
		}

		context.putImageData(newImg,0,0,dirty_x,0,imgWidth-dirty_x, canvas.height);
		context.clearRect(newImg.width, 0, 1, canvas.height);
	};

	var remove_row_fast = function(path){
		// Find min and max x values of seam
		var min_x = Number.MAX_VALUE;
		var max_x = Number.MIN_VALUE;

		for (var p=0; p<path.length; p++) {
			min_x = Math.min(min_x, path[p].index);
			max_x = Math.max(max_x, path[p].index);
		}
		// single dimension array of RGBA
		var midImgData = context.getImageData(min_x, 0, max_x-min_x+1, imgHeight);
		var newImg = context.createImageData(max_x-min_x, imgHeight);

		//Remove seam from middle part
		var new_index=0;
		var old_index = 0;
		for (var y=0; y<imgHeight; y++){
			for (var x=min_x; x<=max_x; x++){
				if (path[y].index === x){
					old_index++;
					continue;
				}
				newImg.data[4*new_index] = midImgData.data[4*old_index];
				newImg.data[4*new_index+1] = midImgData.data[4*old_index+1];
				newImg.data[4*new_index+2] = midImgData.data[4*old_index+2];
				newImg.data[4*new_index+3] = midImgData.data[4*old_index+3];
				new_index++;
				old_index++;
			}
		}

        // put new image data in right place
		context.putImageData(newImg,min_x,0);

		//shift clean data on right side of seam over 1 pixel
		var rightImgData = context.getImageData(max_x+1, 0, imgWidth-max_x-1, imgHeight);
		context.putImageData(rightImgData,max_x,0);

		imgWidth -= 1;
		context.clearRect(imgWidth, 0, 1, canvas.height);
	};

	var add_row = function(path){
		var imgData = context.getImageData(0, 0, imgWidth, imgHeight); // single dimension array of RGBA
		imgWidth += 1;
		var newImg = context.createImageData(imgWidth, imgHeight);
		var path_index = 0;
		var new_index = 0;
		var dirty_x = 0;
		for (var i=0; i < imgData.data.length/4; i+=1){
			if (path[path_index].getIndex(imgWidth-1,path_index) === i){
				dirty_x = Math.min(dirty_x, path[path_index].index);
				newImg.data[4*new_index] = path[path_index].r;
				//Highlight seams by order of removal
				if (seam_highlight) {
					newImg.data[4*new_index] += lod;
					newImg.data[4*new_index] = Math.min(newImg.data[4*new_index],255);
				}
				newImg.data[4*new_index+1] = path[path_index].g;
				newImg.data[4*new_index+2] = path[path_index].b;
				newImg.data[4*new_index+3] = 255;
				path_index = Math.min(path_index+1,path.length-1);
				new_index++;
			}
			newImg.data[4*new_index] = imgData.data[4*i];
			newImg.data[4*new_index+1] = imgData.data[4*i+1];
			newImg.data[4*new_index+2] = imgData.data[4*i+2];
			newImg.data[4*new_index+3] = imgData.data[4*i+3];
			new_index++;
		}
		context.putImageData(newImg,0,0,dirty_x,0,imgWidth-dirty_x, canvas.height);
	};

	var add_row_fast = function(path){
		// Find min and max x values of seam
		var min_x = Number.MAX_VALUE;
		var max_x = Number.MIN_VALUE;

		for (var p=0; p<path.length; p++) {
			min_x = Math.min(min_x, path[p].index);
			max_x = Math.max(max_x, path[p].index);
		}
		min_x--;
		// single dimension array of RGBA
		var midImgData = context.getImageData(min_x, 0, max_x-min_x+1, imgHeight);
		var newImg = context.createImageData(max_x-min_x+2, imgHeight);

		//Remove seam from middle part
		var new_index=0;
		var old_index = 0;
		for (var y=0; y<imgHeight; y++){
			for (var x=min_x; x<=max_x; x++){
				if (path[y].index === x){
					if (lod<1) {
						var offset = 4*(old_index-1);
						var red = midImgData.data[offset] + midImgData.data[4*old_index];
						red /= 2;
						var green = midImgData.data[offset+1] + midImgData.data[4*old_index+1];
						green /= 2;
						var blue = midImgData.data[offset+2] + midImgData.data[4*old_index+2];
						blue /= 2;
						
						newImg.data[4*new_index] = red;
						newImg.data[4*new_index+1] = green;
						newImg.data[4*new_index+2] = blue;
						newImg.data[4*new_index+3] = 255;
					} else {
						newImg.data[4*new_index] = path[y].r;
						newImg.data[4*new_index+1] = path[y].g;
						newImg.data[4*new_index+2] = path[y].b;
						newImg.data[4*new_index+3] = 255;
					}
					new_index++;
				}
				newImg.data[4*new_index] = midImgData.data[4*old_index];
				newImg.data[4*new_index+1] = midImgData.data[4*old_index+1];
				newImg.data[4*new_index+2] = midImgData.data[4*old_index+2];
				newImg.data[4*new_index+3] = midImgData.data[4*old_index+3];
				new_index++;
				old_index++;
			}
		}

		var rightImgData = context.getImageData(max_x+1, 0, imgWidth-max_x, imgHeight);
		context.putImageData(rightImgData,max_x+2,0);

        // put new image data in right place
		context.putImageData(newImg,min_x,0);

		imgWidth += 1;
	};

    var remove_col = function(path) {
        var imgData = horizcontext.getImageData(0, 0, horizImgWidth, horizImgHeight);
        horizImgHeight -= 1;
        var newImg = horizcontext.createImageData(horizImgWidth, horizImgHeight);

        var path_index = 0;
        var new_index = 0;

        for (var x=0; x < imgData.width; x+=1) {
            for (var y=0; y < imgData.height; y+=1) {
                if (path[x].index <= y) { // if path pixel is at or above where we are now
                    newImg.data[4*(y*horizImgWidth+x)]   = imgData.data[4*((y+1)*horizImgWidth+x)];
                    newImg.data[4*(y*horizImgWidth+x)+1] = imgData.data[4*((y+1)*horizImgWidth+x)+1];
                    newImg.data[4*(y*horizImgWidth+x)+2] = imgData.data[4*((y+1)*horizImgWidth+x)+2];
                    newImg.data[4*(y*horizImgWidth+x)+3] = imgData.data[4*((y+1)*horizImgWidth+x)+3];
                } else {
                    newImg.data[4*(y*horizImgWidth+x)]   = imgData.data[4*(y*horizImgWidth+x)];
                    newImg.data[4*(y*horizImgWidth+x)+1] = imgData.data[4*(y*horizImgWidth+x)+1];
                    newImg.data[4*(y*horizImgWidth+x)+2] = imgData.data[4*(y*horizImgWidth+x)+2];
                    newImg.data[4*(y*horizImgWidth+x)+3] = imgData.data[4*(y*horizImgWidth+x)+3];
                }
            }
        }

        horizcontext.clearRect(0, 0, horizcanvas.width, horizcanvas.height);
        horizcontext.putImageData(newImg, 0, 0);

    };

    var add_col = function(path) {
        var imgData = horizcontext.getImageData(0, 0, horizImgWidth, horizImgHeight);
        horizImgHeight += 1;
        var newImg = horizcontext.createImageData(horizImgWidth, horizImgHeight);
        var originaly = 0;
        var newy = 0;

        for (var x=0; x < horizImgWidth; x+=1) {
            originaly = 0;
            newy = 0;
            for (var y=0; y < horizImgHeight; y+=1) {
                if (path[x].index === newy) { // if path pixel is at where we are
                    newImg.data[4*(newy*horizImgWidth+x)]   = 255;
                    newImg.data[4*(newy*horizImgWidth+x)+1] = 0;
                    newImg.data[4*(newy*horizImgWidth+x)+2] = 0;
                    newImg.data[4*(newy*horizImgWidth+x)+3] = 255;
                    newy += 1;
                }
                newImg.data[4*(newy*horizImgWidth+x)]   = imgData.data[4*(originaly*horizImgWidth+x)];
                newImg.data[4*(newy*horizImgWidth+x)+1] = imgData.data[4*(originaly*horizImgWidth+x)+1];
                newImg.data[4*(newy*horizImgWidth+x)+2] = imgData.data[4*(originaly*horizImgWidth+x)+2];
                newImg.data[4*(newy*horizImgWidth+x)+3] = imgData.data[4*(originaly*horizImgWidth+x)+3];
                newy += 1;
                originaly += 1;
            }
        }

        horizcontext.clearRect(0, 0, horizcanvas.width, horizcanvas.height);
        horizcontext.putImageData(newImg, 0, 0);

    };


	var down_lod = function(times) {
		for (var i=0; i<times; i++){
			if (lod>=cut_seams.length) break;
			seam = cut_seams[lod];
			lod++;
			//remove_row(seam);
			remove_row_fast(seam);
		}
	};

	var up_lod = function(times) {
		for (var i=0; i<times; i++){
			if (lod <= -(cut_seams.length/2-1)) break;
			lod--;
			seam = cut_seams[lod];
			//add_row(seam);
			add_row_fast(seam);
		}
	};

    var down_horiz_lod = function(times) {
        for (var i=0; i<times; i++) {
            if (horiz_lod >= horiz_cut_seams.length) break;
            seam = horiz_cut_seams[horiz_lod];
            horiz_lod++;
            remove_col(seam);
        }
    };

    var up_horiz_lod = function(times) {
        for (var i=0; i<times; i++) {
            if (horiz_lod < 1) break;
            horiz_lod--;
            seam = horiz_cut_seams[horiz_lod];
            add_col(seam);
        }
    };

});
