#include "software_renderer.h"


#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include<stdio.h>

#include "triangulation.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //



void SoftwareRendererImp::draw_svg( SVG& svg ) {

    // set top level transformation;
    transformation= svg_2_screen;
    //transformation for SVG Size
    list.push_front(svg_2_screen);
    // draw all elements
    for ( size_t i = 0; i < svg.elements.size(); ++i ) {
    draw_element(svg.elements[i]);
  }
  // draw canvas outline

  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y--;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y++;
  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
    resolve();
}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;

}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->render_target = render_target;
  this->target_w = width;
  this->target_h = height;

}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 5 (part 1):
  // Modify this to implement the transformation stack
    // List helps us to iterate through the transformation elements for each object by inserting and removing transform attribute into it
    list.push_front(list.front()*element->transform);
    // The transform attribute for that element is written into transformation which will perform necessary transformation and convert it into x , y , 1 co -ordinates
    transformation=list.front();
    switch(element->type) {
    case POINT:
      draw_point(static_cast<Point&>(*element));

      break;
    case LINE:
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon&>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group&>(*element));
      break;
    default:
      break;
  }
  //removes the transformation that has been performed from the list
  list.pop_front();
}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
        rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit 

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

  // fill in the nearest pixel
  int sx = (int) floor(x);
  int sy = (int) floor(y);

  // check bounds
  if ( sx < 0 || sx >= target_w ) return;
  if ( sy < 0 || sy >= target_h ) return;

  // fill sample - NOT doing alpha blending!
  render_target[4 * (sx + sy * target_w)    ] = (uint8_t) (color.r * 255);
  render_target[4 * (sx + sy * target_w) + 1] = (uint8_t) (color.g * 255);
  render_target[4 * (sx + sy * target_w) + 2] = (uint8_t) (color.b * 255);
  render_target[4 * (sx + sy * target_w) + 3] = (uint8_t) (color.a * 255);

}

void SoftwareRendererImp::rasterize_line( float x0, float y0, float x1, float y1, Color color) {


    float pk1,pk2=0,del_x,del_y, slope,x,y,temp1,temp2;
    int i;
    //swapping points so that x0 is lesser than x1
    if(x0>x1)
    {
        temp1=x0;
        x0=x1;
        x1=temp1;
        temp2=y0;
        y0=y1;
        y1=temp2;

    }
    x=x0;
    y=y0;
    del_x= x1-x0;
    del_y= y1-y0;
    //computing slope
    slope =del_y/del_x;
    rasterize_point(x,y,color);

    //Initial computation of decision parameter
    pk1=(2*del_y)- del_x ;

    //for slope from 0 to 1
    if(slope>=0 && slope<=1)
    {
        for (i = 0; i < del_x; i++)
        {
            //If pk less than 0 point is x+1 and y
            if (pk1 < 0)
            {
                pk2 = pk1+ (2 * del_y);
                x = x + 1;
                rasterize_point(x, y, color);
            }
                //If pk greater than 0 point is x+1 and y+1
            else if (pk1 > 0)
            {
                pk2 = pk1 + (2 * del_y) - (2 * del_x);
                x = x + 1;
                y = y + 1;
                rasterize_point(x, y, color);
            }

            pk1 = pk2;
        }
    }
    //reinitialization for other slopes
    pk1=(2*del_y)- del_x ;

    //slope between 0 and -1
    if(slope<0 && slope>=-1)
    {
        for (i = 0; i < del_x; i++) {
            if (pk1 < 0)
            {
                pk2 = pk1+ (2 * abs(del_y));
                x = x + 1;
                rasterize_point(x, y, color);
            }
            // Y gets decremented
            else if (pk1 > 0) {
                pk2 = pk1 + abs((2 * del_y))- abs((2 * del_x));
                x = x + 1;
                y = y - 1;
                rasterize_point(x, y, color);
            }
            pk1 = pk2;
        }
    }
    pk1=(2*del_y)- del_x ;

    //slope greater than 1
    if(slope>1)
    {
        //similar to slope between 0 and 1 but y here increases faster than x
        for (i = 0; i < abs(del_y); i++) {
            if (pk1 <= 0)
            {   //increment y if pk is less than 0
                pk2 = pk1 + (2 * del_x);
                y = y + 1;
                rasterize_point(x, y, color);
            }else if (pk1 > 0) {
                pk2 = pk1 + (2 * del_x) - (2 * del_y);
                //increment y and x if pk is greater than 0
                x = x + 1;
                y = y + 1;
                rasterize_point(x, y, color);
            }
            pk1 = pk2;
        }
    }

    pk1=(2*del_y)- del_x;

    //slope less than -1
    if(slope<-1)
    {
        for (i = 0; i < abs(del_y); i++)
        {
            if (pk1 < 0)
            {
                //decrement y if pk is less than 0
                pk2 = pk1 + abs(2 * del_x);
                y = y - 1;
                rasterize_point(x, y, color);
            }
            else if (pk1 > 0)
            {
                //increment x and decrement y if pk is greater than 0
                pk2 = pk1 + abs (2 * del_x) - abs(2 * del_y);
                x = x + 1;
                y = y - 1;
                rasterize_point(x, y, color);
            }
            pk1 = pk2;
        }
    }
}


//function to check if point is in triangle
int check_point(int x, int y,int x0,int y0,int x1, int y1,int x2, int y2)
{

    //from triangulate.cpp
    float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    float cCROSSap, bCROSScp, aCROSSbp;

    ax = x2 - x1;  ay = y2 - y1;
    bx = x0 - x2;  by = y0 - y2;
    cx = x1 - x0;  cy = y1 - y0;
    apx= x - x0;  apy= y - y0;
    bpx= x - x1;  bpy= y - y1;
    cpx= x - x2;  cpy= y - y2;

    aCROSSbp = ax*bpy - ay*bpx;
    cCROSSap = cx*apy - cy*apx;
    bCROSScp = bx*cpy - by*cpx;
    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
 }

void SoftwareRendererImp::rasterize_triangle( float x0, float y0, float x1, float y1, float x2, float y2, Color color ) {
    // Task 3:
    // Implement triangle rasterization
    int i,j;
    float in;
    //creating edges
    rasterize_line(x0,y0,x2,y2,color);
    rasterize_line(x1,y1,x2,y2,color);
    rasterize_line(x0,y0,x1,y1,color);
    //The below nested loop checks if a point is in the triangle by considering a small portion of the entire screen. This portion can be visualized as a square or a rectangle
    for(i=min(x0,min(x1,x2)); i<max(x0,max(x1,x2));i++) // From the point with least x value to the greatest
    {
        for(j=min(y0,min(y1,y2)); j<max(y0,max(y1,y2));j++) // From the point with least y value to the greatest
        {

            in=check_point(i,j,x0,y0,x1,y1,x2,y2);
            //rasterize point only if the point is in the triangle
            if(in>0)
            {
                rasterize_point(i,j,color);
            }

        }
    }

}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 6:
  // Implement image rasterization

}

// resolve samples to render target
void SoftwareRendererImp::resolve( void ) {

  // Task 4:
  // Implement supersampling
  // You may also need to modify other functions marked with "Task 4".
  return;

}


} // namespace CMU462
