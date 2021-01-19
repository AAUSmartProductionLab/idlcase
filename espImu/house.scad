sensorWidth = 20.8;
sensorDepth = 14.5;
sensorHeight = 5; // height from the tips of solderings on the bottom up to the highest component on the top
spaceing = 3;

pcbClerance = 1.8; // clearance underneath the pcb. Determines the height of the pcb standoffs.
pcbHoles = [[2.6,11.8],[17.9, 11.8]];  // relative to lower left corner
pcbHoleDia = 2.6;
pcbThickness = 1.5;

boxWidth = sensorWidth+2*spaceing;
boxDepth = sensorDepth+2*spaceing;
boxHeight = sensorHeight+spaceing;

wallThickness = 1.6;

lidHeight = 2;
lSlidWidth = 3.5;

clipThickness = 1.2;
clipHeight = 3.5;
clipTolerance = 0.1;

$fn = 40;


bottom();
translate([0,0,boxHeight+lidHeight+wallThickness*2])rotate([180,0,180])!lid();
translate([-sensorWidth/2,-sensorDepth/2,wallThickness])pcbDummy();


module bottom(){
    difference(){
        linear_extrude(boxHeight+wallThickness)offset(wallThickness)square([boxWidth,boxDepth], center = true);
        translate([0,0,boxHeight/2+wallThickness])cube([boxWidth,boxDepth,boxHeight+1], center = true);
        // clip holes are placed relative to the top left and right coron of the box. 
        translate([-boxWidth/2,  boxDepth/2, boxHeight+wallThickness])rotate([180,0,0])clipHole();
        translate([boxWidth/2,   boxDepth/2, boxHeight+wallThickness])rotate([180,0,0])mirror([1,0,0])clipHole();
    }

    //standoffs
    translate([-sensorWidth/2, -sensorDepth/2,wallThickness])standoffs();

    //support under pins
    translate([-sensorWidth/2, -sensorDepth/2,wallThickness])cube([sensorWidth,1.2,pcbClerance-0.8]);


}

// lid
module lid(){
    difference(){
        linear_extrude(lidHeight+wallThickness)offset(wallThickness)square([boxWidth,boxDepth], center = true);
        translate([-boxWidth/2,-boxDepth/2,wallThickness])cube([boxWidth,boxDepth,boxHeight+1]);
        translate([-boxWidth/2, -boxDepth/2+spaceing, -100])cube([boxWidth,lSlidWidth,200]);
    }
    // clips
    difference(){
        union(){
            translate([-boxWidth/2,-boxDepth/2, wallThickness])clip();
            translate([boxWidth/2,-boxDepth/2, wallThickness])mirror([1,0,0])clip();
        }
        translate([-boxWidth, -boxDepth/2+spaceing, -100])cube([boxWidth*2,lSlidWidth,200]);

    }
    mirror([1,0,0])
    translate([-sensorWidth/2, -sensorDepth/2,wallThickness])
    for(i = pcbHoles){
        translate(i)cylinder(d=pcbHoleDia+1,h=lidHeight+boxHeight-pcbClerance-pcbThickness);      
    }     
    

}


module clip(){
    length = boxDepth-1.6;

    translate([0,0.8]){    
        rotate([-90,0,0])linear_extrude(length)polygon([[0,0],[0.8,0],[0.8,-0.8]]);
        translate([0.8,0,0])cube([clipThickness,length,lidHeight+clipHeight]);
        translate([0.8+clipThickness,0,0])rotate([-90,0,0])linear_extrude(length)polygon([[0,0],[0.8,0],[0,-0.8]]);
        translate([0.8,0,lidHeight+clipHeight])rotate([-90,0,0,])
            linear_extrude(length)
                polygon([[0,0],[-1.2,0.8*2],[0,0.8*4]]);
    }
}

module clipHole(){
    translate([0.8-clipTolerance,0,clipHeight])rotate([-90,0,0,])
        linear_extrude(boxDepth)
                polygon([[0,0],[-1.2,0.8*2],[0,0.8*4]]);
}

module pcbDummy(){
    if ($preview){
        color("green")
        translate([0,0,pcbClerance])
        linear_extrude(sensorHeight-pcbClerance)difference(){
            square([sensorWidth,sensorDepth]);
            for(i = pcbHoles){
                translate(i)circle(d=pcbHoleDia);
            }
        }
    }
}

module standoffs(){
    for(i = pcbHoles){
        linear_extrude(pcbClerance+pcbThickness){
            translate(i)circle(d=pcbHoleDia); 
        } 
        linear_extrude(pcbClerance){
            difference(){
                translate(i)circle(d=pcbHoleDia+2);
                translate(i)circle(d=pcbHoleDia-1);
            }    
        }
    }
}