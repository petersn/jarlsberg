 // Surface class header

#ifndef _SURFACES_HEADER_
#define _SURFACES_HEADER_

typedef int ivectype;
typedef float fvectype;

template <typename Numeric>
class vector {
  public:
    Numeric x, y, z;

    vector (Numeric xx = 0, Numeric yy = 0, Numeric zz = 0);

    vector operator + (vector);
    vector operator += (vector);
    vector operator - (vector);
    vector operator - (); // Negation
    double operator ~ (); // Euclidean norm
    Numeric operator * (vector); // Dot product
    vector operator * (Numeric); // Scalar product
    vector operator / (Numeric); // Scalar division
    vector operator ^ (vector); // Cross product

    bool operator == (vector);
    bool operator != (vector);
    ostream& operator << (ostream& o);

};

template <typename Numeric>
class matrix {
  public:
    vector<Numeric> row0, row1, row2;

    matrix (vector<Numeric> r0 = vector<Numeric>(1,0,0), vector<Numeric> r1 = vector<Numeric>(0,1,0), vector<Numeric> r2 = vector<Numeric>(0,0,1)) {
        row0 = r0; row1 = r1; row2 = r2;
    }

    vector<Numeric> operator * (vector<Numeric>); // Project
    //matrix<Numeric> operator ~ (); // Transpose
    matrix& operator ~ ();
    ostream& operator << (ostream& o);

};

const vector<ivectype> good_direction = vector<ivectype>(1, 1, 1);

#define pos(x) ((x).local_position)

class surface {
  public:
    matrix<ivectype> localize_basis;
    matrix<ivectype> globalize_basis;
    vector<ivectype> position, local_position;
    ivectype width, height;

      // Empty constructor allowed
    surface () {
    }

    surface ( vector<ivectype> pos, ivectype n_width, ivectype n_height, vector<ivectype> w_axis, vector<ivectype> h_axis ) {
        reinit( pos, n_width, n_height, w_axis, h_axis );
    }

    void reinit ( vector<ivectype> pos, ivectype n_width, ivectype n_height, vector<ivectype> w_axis, vector<ivectype> h_axis ) {

           // Handle negative width and height, relocating the surface to keep them positive
           // Note: Negative width and height are NOT interpreted to flip the surface's normal!
         if (n_width < 0) {
            pos = pos + w_axis * n_width;
            n_width = -n_width;
         }
         if (n_height < 0) {
            pos = pos + h_axis * n_height;
            n_height = -n_height;
         }

        vector<ivectype> temp;
        ivectype scalar_temp;
        position = pos;
        width = n_width;
        height = n_height;

          // Correct the basis, if any vectors point into the (-1,-1,-1) octant
          // (purely arbitrary which octant I ban, but one of them must be banned)
        for (int ii=0; ii<2; ii++) {
            if ( w_axis * good_direction < 0 ) {
                temp   = -w_axis;
                w_axis = h_axis;
                h_axis = temp;

                temp     = h_axis * width;
                position = position - temp;

                scalar_temp = width;
                width  = height;
                height = scalar_temp;
            }
            if ( h_axis * good_direction < 0 ) {
                temp   = -h_axis;
                h_axis = w_axis;
                w_axis = temp;

                temp     = w_axis * height;
                position = position - temp;

                scalar_temp = width;
                width  = height;
                height = scalar_temp;
            }
        }

        localize_basis.row0 = w_axis;
        localize_basis.row1 = h_axis;
        localize_basis.row2 = (w_axis ^ h_axis);

        globalize_basis = localize_basis;
        ~globalize_basis;

        local_position = localize_basis * position;

    }

    ostream& operator << (ostream& o);

};

void surface_local( surface& reference, surface& result, ivectype x, ivectype y, ivectype width, ivectype height ) {

    vector<int> newxyz;

    newxyz = reference.globalize_basis * vector<int>(x, y, reference.local_position.z);

    result.reinit( newxyz, width, height, reference.localize_basis.row0, reference.localize_basis.row1 );

}

#endif

