sensorWidth = 20.8;
sensorDepth = 14.5;
sensorHeight = 5; // height from the tips of solderings on the bottom up to the highest component on the top
spaceing = 1.5;

pcbClerance = 1.8; // clearance underneath the pcb. Determines the height of the pcb standoffs.
pcbHoles = [[2.6,11.8],[17.9, 11.8]];  // relative to lower left corner
pcbHoleDia = 3;
pcbThickness = 1.5;

boxWidth = sensorWidth+2*spaceing;
boxDepth = sensorDepth+2*spaceing;
boxHeight = sensorHeight+spaceing;

wallThickness = 1.2;

lidHeight = 2;
lSlidWidth = 3;

clipThickness = 0.5;
clipHeight = 2;
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
    cube([clipThickness,boxDepth,lidHeight/2]);
    translate([clipThickness,0,0])cube([clipThickness,boxDepth,lidHeight+clipHeight]);
    translate([clipThickness,0,lidHeight+clipHeight])rotate([-90,0,0,])
        linear_extrude(boxDepth)
            polygon([[0,0],[-clipThickness*2,clipThickness],[0,clipThickness*2]]);


}

module clipHole(){
    translate([clipThickness-clipTolerance,0,clipHeight])rotate([-90,0,0,])
        linear_extrude(boxDepth)
            polygon([[0,0],[-clipThickness*2,clipThickness],[0,clipThickness*2]]);

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
            difference(){
                translate(i)circle(d=pcbHoleDia);
                translate(i)circle(d=pcbHoleDia-1);
            }    
        } 
        linear_extrude(pcbClerance){
            difference(){
                translate(i)circle(d=pcbHoleDia+2);
                translate(i)circle(d=pcbHoleDia-1);
            }    
        }
    }
}