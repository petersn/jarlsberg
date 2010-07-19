 // Surfaces header
using namespace std;
#include <iostream>
#include "surfaces.h"
#include "main.h"

  // Vector functions
template <typename NumericA, typename NumericB>
vector<NumericB> vector_recast(vector<NumericA> a) {
	return vector<NumericB>((NumericB) a.x, (NumericB) a.y, (NumericB) a.z);
}

template <typename Numeric>
vector<Numeric>::vector (Numeric xx, Numeric yy, Numeric zz) {
	x = xx; y = yy; z = zz;
}

template <typename Numeric>
vector<Numeric> vector<Numeric>::operator + (vector<Numeric> vect) {
	return vector<Numeric>(x+vect.x, y+vect.y, z+vect.z);
}

template <typename Numeric>
vector<Numeric> vector<Numeric>::operator += (vector<Numeric> vect) {
	this.x += vect.x; this.y += vect.y; this.z += vect.z;
	return vect;
}

template <typename Numeric>
vector<Numeric> vector<Numeric>::operator - (vector<Numeric> vect) {
	return vector<Numeric>(x-vect.x, y-vect.y, z-vect.z);
}

  // Negation
template <typename Numeric>
vector<Numeric> vector<Numeric>::operator - () {
	return vector<Numeric>(-x, -y, -z);
}

  // Euclidean norm
template <typename Numeric>
double vector<Numeric>::operator ~ () {
	return sqrt( x*x + y*y + z*z );
}

  // Dot product
template <typename Numeric>
Numeric vector<Numeric>::operator * (vector<Numeric> vect) {
	return (x*vect.x) + (y*vect.y) + (z*vect.z);
}

  // Scalar product
template <typename Numeric>
vector<Numeric> vector<Numeric>::operator * (Numeric scalar) {
	return vector<Numeric>(x*scalar, y*scalar, z*scalar);
}

  // Scalar division
template <typename Numeric>
vector<Numeric> vector<Numeric>::operator / (Numeric scalar) {
	return vector<Numeric>(x/scalar, y/scalar, z/scalar);
}

  // Cross product
template <typename Numeric>
vector<Numeric> vector<Numeric>::operator ^ (vector<Numeric> vect) {
	return vector<Numeric>(y*vect.z - z*vect.y,
						   z*vect.x - x*vect.z,
						   x*vect.y - y*vect.x);
}

template <typename Numeric>
ostream& operator << (ostream& o, vector<Numeric> a) {
	o << "(" << (a.x) << ", " << (a.y) << ", " << (a.z) << ")";
	return o;
}

template <typename Numeric>
bool vector<Numeric>::operator == (vector<Numeric> b) {
	if (x == b.x && y == b.y && z == b.z) {
		return true;
	}
	else return false;
}

template <typename Numeric>
bool vector<Numeric>::operator != (vector<Numeric> b) {
	if (x == b.x && y == b.y && z == b.z) {
		return false;
	}
	else return true;
}

  // Matrix functions
template <typename NumericA, typename NumericB>
matrix<NumericB> matrix_recast(const matrix<NumericA>& m) {
	return matrix<NumericB>(vector_recast<NumericA, NumericB>(m.row0),
							vector_recast<NumericA, NumericB>(m.row1),
							vector_recast<NumericA, NumericB>(m.row2));
}

  // Project
template <typename Numeric>
vector<Numeric> matrix<Numeric>::operator * (vector<Numeric> vect) {
	return vector<Numeric>( row0 * vect, row1 * vect, row2 * vect );
}

  // Transpose
template <typename Numeric>
matrix<Numeric>& matrix<Numeric>::operator ~ () {
	Numeric tmp;

	tmp = row1.x;
	row1.x = row0.y;
	row0.y = tmp;

	tmp = row2.x;
	row2.x = row0.z;
	row0.z = tmp;

	tmp = row2.y;
	row2.y = row1.z;
	row1.z = tmp;
	return *this;
}

template <typename Numeric>
ostream& operator << (ostream& o, matrix<Numeric> m) {
	o << "{ " << m.row0 << endl;
	o << "  " << m.row1 << endl;
	o << "  " << m.row2 << " }" << endl;
	return o;
}

  // Surface functions
ostream& operator << (ostream& o, surface& s) {
	o << s.localize_basis << s.globalize_basis;
	o << "Position: " << s.position << endl;
	o << "Local position: " << s.local_position << endl;
	o << "Width: " << s.width << " Height: " << s.height << endl;
	return o;
}

template <typename Numeric>
ostream& dump_to_file( ostream& o, vector<Numeric>& s ) {
	o << s.x << "," << s.y << "," << s.z << ".";
	return o;
}

ostream& dump_to_file( ostream& o, int x ) {
	o << x << ";";
	return o;
}

ostream& dump_to_file( ostream& o, double x ) {
	o << x << ";";
	return o;
}

ifstream& operator >> ( ifstream& i, camera_t& c ) {
	i >> c.x; i.ignore(1);
	i >> c.y; i.ignore(1);
	i >> c.z; i.ignore(1);
	i >> c.momentum.x; i.ignore(1);
	i >> c.momentum.y; i.ignore(1);
	i >> c.momentum.z; i.ignore(1);
	i >> c.facing; i.ignore(1);
	i >> c.tilt; i.ignore(1);
	i >> c.floored; i.ignore(1);
	i >> c.facing_target; i.ignore(1);
	i >> c.tilt_target; i.ignore(1);
	i.ignore(1);
	return i;
}

ostream& dump_to_file( ostream& o, surface& s ) {
	  // Store only minimal information, and reconstruct the rest
	dump_to_file( o, s.localize_basis.row0 );
	dump_to_file( o, s.localize_basis.row1 );
	dump_to_file( o, s.position );
	dump_to_file( o, s.width );
	dump_to_file( o, s.height );
	dump_to_file( o, s.surface_texture_index );
	o << endl;
	return o;
}

ostream& dump_to_file( ostream& o, camera_t& c ) {
	dump_to_file( o, c.x );
	dump_to_file( o, c.y );
	dump_to_file( o, c.z );
	dump_to_file( o, c.momentum.x );
	dump_to_file( o, c.momentum.y );
	dump_to_file( o, c.momentum.z );
	dump_to_file( o, c.facing );
	dump_to_file( o, c.tilt );
	dump_to_file( o, c.floored );
	dump_to_file( o, c.facing_target );
	dump_to_file( o, c.tilt_target );
	o << endl;
	return o;
}

ostream& dump_to_file( ostream& o, list<surface>& l ) {
	list<surface>::iterator iter_surf;
	for (iter_surf=l.begin(); iter_surf != l.end(); ++iter_surf) {
		dump_to_file( o, *iter_surf );
	}
	return o;
}

list<surface> load_from_file( ifstream& input ) {
	list<surface> surfaces;
	surface scratch;
	int total = 0;

	while ((! input.eof()) && input.good()) {
		input >> scratch.localize_basis.row0.x; input.ignore(1);
		input >> scratch.localize_basis.row0.y; input.ignore(1);
		input >> scratch.localize_basis.row0.z; input.ignore(1);
		input >> scratch.localize_basis.row1.x; input.ignore(1);
		input >> scratch.localize_basis.row1.y; input.ignore(1);
		input >> scratch.localize_basis.row1.z; input.ignore(1);
		scratch.localize_basis.row2 = scratch.localize_basis.row0 ^ scratch.localize_basis.row1;
		scratch.globalize_basis = scratch.localize_basis;
		~scratch.globalize_basis;
		input >> scratch.position.x; input.ignore(1);
		input >> scratch.position.y; input.ignore(1);
		input >> scratch.position.z; input.ignore(1);
		scratch.local_position = scratch.localize_basis * scratch.position;
		input >> scratch.width; input.ignore(1);
		input >> scratch.height; input.ignore(1);
		input >> scratch.surface_texture_index; input.ignore(1);
		input.ignore(1);
		surfaces.push_back( scratch );
		total++;
	}

	cout << "Read " << total << " surfaces." << endl;

	return surfaces;
}

void render_surface( surface& s ) {
	vector<double> temp = vector_recast<int, double>(s.position);

	// Skip rendering if the magic maximum texture index is given
	if (s.surface_texture_index == global_texture_index) return;

	double scale_factor = global_texture_table[s.surface_texture_index].scale_factor;
#define texCoord(xoff, yoff) glTexCoord2f((s.local_position.x + (xoff)) * scale_factor, (s.local_position.y + (yoff)) * scale_factor)

	glBindTexture(GL_TEXTURE_2D, global_texture_table[s.surface_texture_index].texture_index);

	glBegin(GL_QUADS);

	glMaterialfv(GL_FRONT, GL_EMISSION,  global_texture_table[s.surface_texture_index].emission);

	//glMaterialfv(GL_FRONT, GL_AMBIENT,   Lighting);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
	//glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
	//glMaterialf (GL_FRONT, GL_SHININESS, shininess);

	texCoord(0.0, 0.0);
	glVertex3f(temp.x, temp.y, temp.z);	// Bottom Left Of The Texture and Quad

	temp = temp + vector_recast<int, double>(s.localize_basis.row0) * s.width;
#ifdef DEBUG_MODE
	glTexCoord2f(1.0, 0.0);
#else
	texCoord(s.width, 0.0);
#endif
	glVertex3f(temp.x, temp.y, temp.z);	// Bottom Left Of The Texture and Quad

	temp = temp + vector_recast<int, double>(s.localize_basis.row1) * s.height;
#ifdef DEBUG_MODE
	glTexCoord2f(1.0, 1.0);
#else
	texCoord(s.width, s.height);
#endif
	glVertex3f(temp.x, temp.y, temp.z);	// Bottom Left Of The Texture and Quad

	temp = temp - vector_recast<int, double>(s.localize_basis.row0) * s.width;
#ifdef DEBUG_MODE
	glTexCoord2f(0.0, 1.0);
#else
	texCoord(0.0, s.height);
#endif
	glVertex3f(temp.x, temp.y, temp.z);	// Bottom Left Of The Texture and Quad

	glEnd();

}

inline bool coplanar( /* const */ surface& a, /* const */ surface& b ) {
	if (! (a.localize_basis.row2 == b.localize_basis.row2) )
		return false;

	if (! (a.local_position.z == b.local_position.z) )
		return false;

	return true;
}

inline bool contains( /* const */ surface& a, /* const */ surface& b ) {
	if (!coplanar(a, b)) return false;

	  // Make sure the other surface is contained in the x axis
	if (!( pos(b).x >= pos(a).x && pos(b).x + b.width <= pos(a).x + a.width ))
		return false;

	  // Make sure the other surface is contained in the y axis
	if (!( pos(b).y >= pos(a).y && pos(b).y + b.height <= pos(a).y + a.height ))
		return false;

	return true;
}

inline bool overlaps( /* const */ surface& s, /* const */ surface& t ) {
	if (!coplanar(s, t)) return false;

	  // Make sure the other surface overlaps in the x axis
	if ( pos(t).x >= pos(s).x + s.width || pos(t).x + t.width <= pos(s).x )
		return false;

	  // Make sure the other surface overlaps in the y axis
	if ( pos(t).y >= pos(s).y + s.height || pos(t).y + t.height <= pos(s).y )
		return false;

	return true;
}

inline bool adjacent( /* const */ surface& s, /* const */ surface& t ) {
	if (!coplanar(s, t)) return false;
	if (overlaps(s, t)) return false;

	  // Return false if no two edges could possibly be shared
	if (!(pos(s).x+s.width == pos(t).x || pos(s).y+s.height == pos(t).y || pos(s).x == pos(t).x+t.width || pos(s).y == pos(t).y+t.height))
		return false;

	 // Finally, do an intersection test, only with the conditions being relaxed by one cell in either x NAND y
	if ((pos(t).x+t.width >= pos(s).x || pos(t).x <= pos(s).x+s.width) && \
		 (pos(t).y+t.height > pos(s).x || pos(t).y < pos(s).x+s.height))
		return true;

	if ((pos(t).y+t.height >= pos(s).x || pos(t).y <= pos(s).x+s.height) && \
		(pos(t).x+t.width > pos(s).x || pos(t).x < pos(s).x+s.width))
		return true;

	return false;
}

const vector<double> no_hit = vector<double>(INFINITY, INFINITY, INFINITY);

vector<double> ray_cast( const surface& s, vector<double> origin, vector<double> direction ) {

	vector<double> fnormal = vector_recast<int, double>( s.localize_basis.row2 );
	vector<double> delta, hitpoint, localhitpoint;
	double t;
	double denominator;

	  // Find the rate of approach
	denominator = direction * fnormal;

	  // If it's zero, then the line is parallel to the surface, and never intersects
	if (denominator == 0.0) {
		return no_hit;
	}

	  // Find the total time to approach
	t = - ( origin * fnormal - ((double)(s.local_position.z)) ) / denominator;

	  // If it's less than zero, then the "ray" being cast doesn't intersect, but the line being cast does. (If you know what I mean)
	if (t < 0.0) {
		return no_hit;
	}

	delta = direction * t;

	hitpoint = origin + delta;

	matrix<double> cast_result = matrix_recast<int, double>( s.localize_basis );

	localhitpoint = cast_result * hitpoint;

	double localx, localy;
	localx = localhitpoint.x;
	localy = localhitpoint.y;

	if (localx < s.local_position.x || localy < s.local_position.y || localx > s.local_position.x+s.width || localy > s.local_position.y+s.height ) {
		return no_hit;
	}

	return hitpoint;

}

  // Returns -1 if main should remain unmodified.
  // Otherwise, returns the number of new surfaces, and dumps them into cluster
  // The resultant surfaces are of main-subtractor
list<surface> subtract( surface& main, surface& subtractor ) {
	list<surface> l;
	surface main_copy;
	surface scratch;

	//cout << "Main" << endl << main;
	//cout << "Subtractor" << endl << subtractor;

	  // Test for trivial cases
	if (!overlaps(main, subtractor)) {
		l.push_back( main );
		return l;
	}
	if (contains(subtractor, main)) {
		return l;
	}

	main_copy = main;

	  // Other surface is to my right, so I pull over to them
	if (pos(subtractor).x > main_copy.local_position.x) {
		surface_local( main_copy, scratch, \
				main_copy.local_position.x, \
				main_copy.local_position.y, \
				pos(subtractor).x-main_copy.local_position.x, \
				main_copy.height \
				);
		l.push_back( scratch );
		main_copy.width -= pos(subtractor).x-main_copy.local_position.x;
		main_copy.local_position.x = pos(subtractor).x;
	}

	  // Other surface is to my left, so I pull over to them
	if (main_copy.local_position.x+main_copy.width > pos(subtractor).x+subtractor.width) {
		surface_local( main_copy, scratch, \
				pos(subtractor).x+subtractor.width, \
				main_copy.local_position.y, \
				(main_copy.local_position.x+main_copy.width)-(pos(subtractor).x+subtractor.width), \
				main_copy.height \
				);
		l.push_back( scratch );
		main_copy.width = pos(subtractor).x+subtractor.width-main_copy.local_position.x;
	}

	  // Early out, if this was sufficient
	  // Required for correctness, so we don't produce some slivers along the y-axis with zero width
	if (main_copy.width == 0) {
		return l;
	}

	  // Other surface is above me, so I drag up to them
	if (pos(subtractor).y > main_copy.local_position.y) {
		//collection.append( self.local_surface( self.x, self.y, self.w, other.y-self.y ) )
		surface_local( main_copy, scratch, \
				main_copy.local_position.x, \
				main_copy.local_position.y, \
				main_copy.width, \
				pos(subtractor).y-main_copy.local_position.y \
				);
		l.push_back( scratch );
		main_copy.height -= pos(subtractor).y - main_copy.local_position.y;
		main_copy.local_position.y = pos(subtractor).y;
	}

	  // Other surface is below me, so I drag down to them
	if (main_copy.local_position.y+main_copy.height > pos(subtractor).y+subtractor.height) {
		//collection.append( self.local_surface( self.x, other.y+other.h, self.w, (self.y+self.h)-(other.y+other.h) ) )
		surface_local( main_copy, scratch, \
				main_copy.local_position.x, \
				pos(subtractor).y+subtractor.height, \
				main_copy.width, \
				(main_copy.local_position.y+main_copy.height)-(pos(subtractor).y+subtractor.height) \
				);
		l.push_back( scratch );
		main_copy.height = pos(subtractor).y + subtractor.height - main_copy.local_position.y;
	}

	return l;
}

list<surface> intersect( surface& s, surface& t ) {
	list<surface> l;

	if (!overlaps(s, t))
		return l;

	surface result;

	int startx, starty, endx, endy;
	startx = pos(s).x; if (pos(t).x > startx) startx = pos(t).x;
	starty = pos(s).y; if (pos(t).y > starty) starty = pos(t).y;
	endx   = pos(s).x + s.width;  if (pos(t).x + t.width < endx)  endx = pos(t).x + t.width;
	endy   = pos(s).y + s.height; if (pos(t).y + t.height < endy) endy = pos(t).y + t.height;

	surface_local( s, result, startx, starty, endx-startx, endy-starty );

	l.push_back( result );

	return l;
}

  // Overloading for easy list concatenation
list<surface> operator += (list<surface>& lhs, list<surface>& rhs) {
	lhs.insert( lhs.end(), rhs.begin(), rhs.end() );
	return lhs;
}

  // Extrudes the part of main that ext overlaps along main's normal by units
  // Returns the number of surfaces to be extruded, and places them into cluster
  // If no change should be made to main, -1 is returned
  // The return value is also placed into cluster_count
list<surface> extrude( surface& main, surface& ext, int zunits ) {
	list<surface> l;

	if (!overlaps(main, ext)) {
		l.push_back( main );
		return l;
	}

	list<surface> temp;

	temp = subtract( main, ext );
	l.insert( l.end(), temp.begin(), temp.end() );

	temp = intersect( main, ext );
	l.insert( l.end(), temp.begin(), temp.end() );

	surface& extruded = l.back();

	//corners = get_corners( extruded );
	vector<int> temp_vector, lowerleft, lowerright, upperleft, upperright;

	temp_vector = extruded.position;

	lowerleft = temp_vector;
	temp_vector = temp_vector + extruded.localize_basis.row0 * extruded.width;
	lowerright = temp_vector;
	temp_vector = temp_vector + extruded.localize_basis.row1 * extruded.height;
	upperright = temp_vector;
	temp_vector = temp_vector - extruded.localize_basis.row0 * extruded.width;
	upperleft = temp_vector;

	extruded.local_position.z += zunits;
	extruded.position = extruded.globalize_basis * extruded.local_position;

			  //# Safe the corners for generating orthogonal surfaces
		//lowerleft, lowerright, upperleft, upperright = extruded.corners()

		  //# Extrude and save
		//extruded = extruded.change_depth( extruded.z + zunits )
		//collection.append( extruded )

		//normal_axis = self.normal
		//if zunits < 0:
			//normal_axis = invert(normal_axis)
			//zunits = -zunits

	vector<int> normal_axis = extruded.localize_basis.row2;
	if (zunits < 0) {
		normal_axis = -normal_axis;
		zunits = -zunits;
	}

		  //# Fill in the orthogonal surfaces
		//surface = Surface( lowerleft[0], lowerleft[1], lowerleft[2], zunits, extruded.h, normal_axis, self.haxis )
		//collection.append( surface )

	surface scratch;

	//printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

	// Bottom

	//surface_init( &(cluster[cluster_count]), corners.lowerleft, -zunits, extruded->height, normal_axis, main->localize_basis.row1 );

	scratch.reinit( lowerleft, zunits, extruded.height, normal_axis, main.localize_basis.row1 );
	scratch.surface_texture_index = main.surface_texture_index;
	l.push_back( scratch );
	//surface_init( &(cluster[cluster_count]), corners.lowerleft, zunits, extruded->height, normal_axis, main->localize_basis.row1 );

	//printsurface( &(cluster[cluster_count]) );
	//count++;
	//cluster_count++;

		//surface = Surface( upperleft[0], upperleft[1], upperleft[2], zunits, extruded.w, normal_axis, self.waxis )
		//collection.append( surface )

	// Left

	//surface_init( &(cluster[cluster_count]), corners.upperleft, -zunits, extruded->width, normal_axis, main->localize_basis.row0 );

	scratch.reinit( upperleft, zunits, extruded.width, normal_axis, main.localize_basis.row0 );
	scratch.surface_texture_index = main.surface_texture_index;
	l.push_back( scratch );
	//surface_init( &(cluster[cluster_count]), corners.upperleft, zunits, extruded->width, normal_axis, main->localize_basis.row0 );


	//printsurface( &(cluster[cluster_count]) );
	//count++;
	//cluster_count++;

		//surface = Surface( upperright[0], upperright[1], upperright[2], zunits, extruded.h, normal_axis, invert(self.haxis) )
		//collection.append( surface )

	// Top

	//surface_init( &(cluster[cluster_count]), corners.upperright, -extruded->height, -zunits, main->localize_basis.row1, normal_axis );

	scratch.reinit( upperright, zunits, extruded.height, normal_axis, -(main.localize_basis.row1) );
	scratch.surface_texture_index = main.surface_texture_index;
	l.push_back( scratch ); // Problematic
	//surface_init( &(cluster[cluster_count]), corners.upperright, zunits, extruded->height, normal_axis, iinvert(main->localize_basis.row1) );


	//printsurface( &(cluster[cluster_count]) );
	//count++;
	//cluster_count++;

		//surface = Surface( lowerright[0], lowerright[1], lowerright[2], zunits, extruded.w, normal_axis, invert(self.waxis) )
		//collection.append( surface )

	// Right

	//surface_init( &(cluster[cluster_count]), corners.lowerright, -zunits, extruded->width, normal_axis, iinvert(main->localize_basis.row0) );

	scratch.reinit( lowerright, zunits, extruded.width, normal_axis, -(main.localize_basis.row0) );
	scratch.surface_texture_index = main.surface_texture_index;
	l.push_back( scratch ); // Problematic
	//surface_init( &(cluster[cluster_count]), corners.lowerright, zunits, extruded->width, normal_axis, iinvert(main->localize_basis.row0) );


	//printsurface( &(cluster[cluster_count]) );
	//count++;
	//cluster_count++;

	return l;
	//return count;
}

void flip_surface( surface& s ) {
	//return Surface( self.absx, self.absy, self.absz, self.h, self.w, self.haxis, self.waxis )
	s.reinit( s.position, s.height, s.width, s.localize_basis.row1, s.localize_basis.row0 );
}

  // Checks all pairs in source, and removes the extraneous portions that are of zero volume
list<surface> prune( list<surface> source ) {
	list<surface> dest;
	list<surface> temp_list;
	list<surface>::iterator iter_surf;
	list<surface>::iterator iter_surf2;
	list<surface>::iterator iter_surf3;
	surface temp;
	bool flag;

	while (1) {
		flag = false;
		for (iter_surf=source.begin(); iter_surf != source.end(); ++iter_surf) {
			for (iter_surf2=source.begin(); iter_surf2 != source.end(); ++iter_surf2) {
				if (iter_surf == iter_surf2) continue;
				if ((*iter_surf).width == 0 || (*iter_surf2).width == 0) continue;
				if ((*iter_surf).localize_basis.row2 == -((*iter_surf2).localize_basis.row2)) {
					temp = *iter_surf2;
					flip_surface( temp );
					if (overlaps( *iter_surf, temp )) {
						temp_list = subtract( *iter_surf, temp );
						source.insert( source.end(), temp_list.begin(), temp_list.end() );
						temp_list = subtract( temp, *iter_surf );
						for (iter_surf3=temp_list.begin(); iter_surf3 != temp_list.end(); ++iter_surf3) {
							flip_surface( *iter_surf3 );
						}
						source.insert( source.end(), temp_list.begin(), temp_list.end() );
						(*iter_surf).width = 0;
						(*iter_surf2).width = 0;
						flag = true;
						break;
					}
				}
				else if ((*iter_surf).localize_basis.row2 == ((*iter_surf2).localize_basis.row2)) {
					temp = *iter_surf2;
					if (overlaps( *iter_surf, temp )) {
						temp_list = subtract( *iter_surf, temp );
						source.insert( source.end(), temp_list.begin(), temp_list.end() );
						(*iter_surf).width = 0;
						//(*iter_surf2).width = 0;
						flag = true;
						break;
					}
				}
			}
			if (flag) break;
		}
		if (!flag) break;
	}

	for (iter_surf=source.begin(); iter_surf != source.end(); ++iter_surf) {
		if ((*iter_surf).width && (*iter_surf).height) {
			dest.push_back( *iter_surf );
		}
	}

	return dest;
}

  // Collapses adjacent and containing surfaces into one
list<surface> optimize( list<surface> source ) {
	list<surface> dest;
	list<surface> temp_list;
	list<surface>::iterator iter_surf;
	list<surface>::iterator iter_surf2;
	list<surface>::iterator iter_surf3;
	surface temp;
	bool flag;

	while (1) {
		flag = false;
		for (iter_surf=source.begin(); iter_surf != source.end(); ++iter_surf) {
			for (iter_surf2=source.begin(); iter_surf2 != source.end(); ++iter_surf2) {
				surface& s = *iter_surf;
				surface& t = *iter_surf2;

				if (iter_surf == iter_surf2) continue;
				if (s.width == 0 || t.width == 0) continue;
				if (!adjacent( s, t )) continue;
				// Guarantee identical textures!
				if (s.surface_texture_index != t.surface_texture_index) continue;

				if (pos(s).y == pos(t).y && s.height == t.height) {
					int start, end, tmp;
					start	= pos(s).x;
					if ((tmp = pos(t).x) < start) start = tmp;
					end	  = pos(s).x+s.width;
					if ((tmp = pos(t).x+t.width) > end) end = tmp;
					surface_local( s, temp, start, pos(s).y, end-start, s.height );
					source.push_back( temp );
					//list_append( source, &temp );
					s.width = 0; s.height = 0;
					t.width = 0; t.height = 0;
					flag = true;
					break;
				}

				else if (pos(s).x == pos(t).x && s.width == t.width) {
					int start, end;
					start = pos(s).y;
					if (pos(t).y < start) start = pos(t).y;
					end   = pos(s).y+s.height;
					if (pos(t).y+t.height > end) end = pos(t).y+t.height;
					surface_local( s, temp, pos(s).x, start, s.width, end-start );
					source.push_back( temp );
					//list_append( dest, &temp );
					s.width = 0; s.height = 0;
					t.width = 0; t.height = 0;
					flag = true;
					break;
				}

				/*
				temp_list = subtract( *iter_surf, temp );
				source.insert( source.end(), temp_list.begin(), temp_list.end() );
				temp_list = subtract( temp, *iter_surf );
				for (iter_surf3=temp_list.begin(); iter_surf3 != temp_list.end(); ++iter_surf3) {
					flip_surface( *iter_surf3 );
				}
				source.insert( source.end(), temp_list.begin(), temp_list.end() );
				(*iter_surf).width = 0;
				(*iter_surf2).width = 0;
				flag = true;
				break;
				*/
			}
			if (flag) break;
		}
		if (!flag) break;
	}

	for (iter_surf=source.begin(); iter_surf != source.end(); ++iter_surf) {
		if ((*iter_surf).width && (*iter_surf).height) {
			dest.push_back( *iter_surf );
		}
	}

	return dest;
}

vector<double> limit( list<surface> source, vector<double> p, vector<double>& momentum, double width, double base, double head, int* floored=NULL ) {
	list<surface>::iterator iter_surf;

	// Initially, you haven't touched the ground yet
//	if (floored != NULL)
//		*floored = 0;

	for (iter_surf=source.begin(); iter_surf != source.end(); ++iter_surf) {
		surface& s = *iter_surf;
		matrix<double>localize_basis = matrix_recast<int, double>( s.localize_basis );
		vector<double>loc_p = localize_basis * p;
		loc_p.z -= s.local_position.z;
		// Test if the surface contains the locally projected point
		if (loc_p.x > s.local_position.x && loc_p.x < s.local_position.x+s.width &&
			loc_p.y > s.local_position.y && loc_p.y < s.local_position.y+s.height) {
			double offset = 0.0;
			if (s.localize_basis.row2.x != 0 || s.localize_basis.row2.z != 0)
				offset = width;
			else if (s.localize_basis.row2.y > 0)
				offset = base;
			else if (s.localize_basis.row2.y < 0)
				offset = head;
			if (loc_p.z > 0 && loc_p.z < offset) {
				matrix<double>globalize_basis = matrix_recast<int, double>( s.globalize_basis );
#ifdef MOMENTUM
				momentum = momentum + (((globalize_basis * vector<double>(loc_p.x, loc_p.y, s.local_position.z+offset)) - p) * CONST_physics_goeyness);
#else
				p = (globalize_basis * vector<double>(loc_p.x, loc_p.y, s.local_position.z+offset));
#endif

				// Test if damage should be dealt by the surface in question
				if (game_mode == GAME_PLAYING) {
					hit_points -= global_texture_table[s.surface_texture_index].damage_per_frame;
					victory_total += global_texture_table[s.surface_texture_index].victory_points;
				}

				// If you're standing on the ground, tell the caller, if they asked
				if (s.localize_basis.row2.y > 0 && floored != NULL) {
					*floored = CONST_jump_grace_period;
					momentum = vector<double>(0.0, 0.0, 0.0);
				}
				if (loc_p.z > 0 && loc_p.z < 0.1) {
					p = globalize_basis * vector<double>(loc_p.x, loc_p.y, s.local_position.z+offset);
				}
			}
		}
	}

	return p;
}

/*
		def choose_function( x ):
			if x[0] != 0 or x[1] != 0:
				return width
			if x[2] > 0:
				return base
			if x[2] < 0:
				return head
		x, y, z = x, z, y#-(offset-climb)
		for surface in self.surfaces:
			lx, ly, depth = surface.project( (x, y, z) )
			if surface.contains_point( (lx, ly) ):
				offset = choose_function( surface.normal )
				if depth > 0 and depth < offset: #offset-climb and depth < offset:
					x, y, z = surface.absolute( (lx, ly), offset )
		return x, z, y
*/

