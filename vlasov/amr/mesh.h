#pragma once

#include <iostream>

#include <cmath> 
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_map>
// #include <map>

// #include "reversed_iterator.h"



namespace toolbox {



/* \brief n-dimensional adaptive mesh 
 *
 * Template for n-dimensional adaptive mesh class.
 * Internally the class uses unordered_map (i.e., hash map)
 * to store the data. Indexing is done via unique keys that follow
 * Morton Z-ordering.
 *
 * TODO: Dimension specialization does not work (giving D has no effect as it is D=3)
 */
template<typename T, int D>
class AdaptiveMesh {

  public:

  typedef std::array<uint64_t, D> indices_t;
  typedef std::array<T, D> value_array_t;

  using iterator       = typename std::unordered_map<uint64_t, T>::iterator;
  using const_iterator = typename std::unordered_map<uint64_t, T>::const_iterator;
  std::unordered_map<uint64_t, T> data;

  // using iterator       = typename std::map<uint64_t, T>::iterator;
  // using const_iterator = typename std::map<uint64_t, T>::const_iterator;
  // std::map<uint64_t, T> data;


  static const uint64_t error_cid = 0;
  static const uint64_t error_index = 0xFFFFFFFFFFFFFFFF;
  int maximum_refinement_level = 10;
	uint64_t last_cid;
  size_t number_of_cells = 0;
  

  indices_t length;

  int current_refinement_level = 0;


  void resize(indices_t given_length)
  {
    length[0] = given_length[0];
    length[1] = given_length[1];
    length[2] = given_length[2];

    update_last_cid();
  }

  void set(uint64_t key, T val)
  {
    data[key] = val;
  }


  T get(const uint64_t key) const
  {
    // return data.at(key);
    const_iterator it = data.find(key);
    return it == data.end() ? T(0) : it->second;
  }


  /*
  // set item
  T operator() (indices_t indx, int refinement_level) 
  {
    uint64_t cid = get_cell_from_indices(indx, refinement_level);
    return data[cid];
  }

  // get item
  const T operator() (indices_t indx, int refinement_level) const
  {
    uint64_t cid = get_cell_from_indices(indx, refinement_level);
    const_iterator it = data.find(cid);
    return it == data.end() ? T(0) : it->second;
  }
  */



	uint64_t get_last_cid() const
	{
		return last_cid;
	}


	void update_last_cid()
	{
		const uint64_t grid_length
			= length[0] * length[1] * length[2];

		last_cid = 0;
		for (int i = 0; i <= maximum_refinement_level; i++) {
			last_cid += grid_length * (uint64_t(1) << (i * 3));
		}
	}
  

  indices_t get_indices(uint64_t cid) const 
  {

    if (cid == error_cid || cid > last_cid) {
      const indices_t error_indices = {{error_index, error_index, error_index}};
      return error_indices;
    }

    // subtract ids of larger cells
    const int refinement_level = get_refinement_level(cid);
    for (int i = 0; i < refinement_level; i++) {
      cid -=
          length[0]
        * length[1]
        * length[2]
        * (uint64_t(1) << (i * 3));
    }

    cid -= 1;	// cell numbering starts from 1
    const indices_t indices = 
    {{
      (cid % (length[0] * (uint64_t(1) << refinement_level))),
     ((cid / (length[0] * (uint64_t(1) << refinement_level)))
           % (length[1] * (uint64_t(1) << refinement_level))),
      (cid / (length[0] * length[1] * (uint64_t(1) << (2 * refinement_level)) ))
    }};

    return indices;
  }


  uint64_t get_cell_from_indices(
      const indices_t& indices,
      const int refinement_level = 0
  ) const 
  { 
		if (indices[0] >= this->length[0] * (uint64_t(1) << this->maximum_refinement_level)) {
			return error_cid;
		}

		if (indices[1] >= this->length[1] * (uint64_t(1) << this->maximum_refinement_level)) {
			return error_cid;
		}

		if (indices[2] >= this->length[2] * (uint64_t(1) << this->maximum_refinement_level)) {
			return error_cid;
		}

		if (refinement_level < 0) {
			return error_cid;
		}

		if (refinement_level > this->maximum_refinement_level) {
			return error_cid;
		}

		// cell numbering starts at 1
		uint64_t cid = 1;

		// add ids of larger cells
		for (int i = 0; i < refinement_level; i++) {
			cid +=
				  this->length[0]
				* this->length[1]
				* this->length[2]
				* (uint64_t(1) << (i * 3));
		}

    const indices_t this_level_length = get_size(refinement_level);

		cid
			+= indices[0] 
			+  indices[1] * this_level_length[0]
			+  indices[2] * this_level_length[0] * this_level_length[1];

      
		return cid;
	}



  int get_refinement_level(const uint64_t cid) const 
  {
		if (cid == error_cid || cid > get_last_cid() ) {
			return -2;
		}

		int refinement_level = 0;
		uint64_t current_last = 0;

		while (refinement_level <= maximum_refinement_level) {
			current_last +=
				  length[0]
				* length[1]
				* length[2]
				* (uint64_t(1) << 3 * refinement_level);

			if (cid <= current_last) {
				break;
			}

			refinement_level++;
		}

		if (refinement_level > maximum_refinement_level) {
			return -1;
		}

		return refinement_level;
	}


  int get_maximum_possible_refinement_level() const
	{
		const uint64_t grid_length
			= length[0] * length[1] * length[2];
		int refinement_level = 0;
		double current_last = 0;
		while (current_last <= double(~uint64_t(0))) {
			// TODO: don't assume 3 dimensions
			current_last += double(grid_length) * std::pow(double(8), double(refinement_level));
			refinement_level++;
		}

		return refinement_level - 2;
	}



  bool set_maximum_refinement_level(const int given_refinement_level)
  {
    if(given_refinement_level < 0) return false;

    maximum_refinement_level = given_refinement_level;

    return true;
  }


  indices_t get_parent_indices(const indices_t& indices) const
  {

    /*
    uint64_t shift = (uint64_t(1) << int(1));
    std::cout << "shift:" << shift << "\n";
    std::cout << "i0: " << indices[0]/2 <<
                 "i1: " << indices[1]/2 <<
                 "i2: " << indices[2]/2 <<
                 "\n";
    */

    // NOTE: implicit int casting does the flooring of this value
    /*
    indices_t parent_indices = 
    {{
       indices[0] / uint64_t(2),
       indices[1] / uint64_t(2),
       indices[2] / uint64_t(2) 
    }};
    */

    indices_t parent_indices = 
    {{
       indices[0] / (uint64_t(1) << 1),
       indices[1] / (uint64_t(1) << 1),
       indices[2] / (uint64_t(1) << 1) 
    }};

    return parent_indices;
  }

  indices_t get_level_0_parent_indices(indices_t& indices, int refinement_level) const
  {
    // uint64_t cid = get_cell_from_indices(indices);
    // int refinement_level = get_refinement_level(cid);
      
    if(refinement_level <= 1) return get_parent_indices(indices);


    indices_t parent_indices = 
    {{
       indices[0] / (uint64_t(1) << (refinement_level)),
       indices[1] / (uint64_t(1) << (refinement_level)),
       indices[2] / (uint64_t(1) << (refinement_level)) 
    }};
    
    return parent_indices;
  }


  uint64_t get_parent(const uint64_t cid) const
	{
		const int refinement_level = get_refinement_level(cid);

		if (refinement_level < 0 || refinement_level > maximum_refinement_level) {
			return error_cid;
		}

		if (refinement_level == 0) {
			return cid;
		}

    // get my current indices
    const indices_t indices = get_indices(cid);
		const indices_t parent_indices = get_parent_indices(indices);

    return get_cell_from_indices(parent_indices);
	}


  uint64_t get_level_0_parent(const uint64_t cid) const 
  {
		const int refinement_level = get_refinement_level(cid);

		if (refinement_level < 0 || refinement_level > maximum_refinement_level) {
			return error_cid;
		}

		if (refinement_level == 0) {
			return cid;
		}

    indices_t 
      indices = get_indices(cid),
		  parent0_indices = get_level_0_parent_indices(indices, refinement_level);

    return get_cell_from_indices(parent0_indices);
  }



  std::vector<uint64_t> get_children(const uint64_t cid) const 
	{
		std::vector<uint64_t> children;

		if (cid == error_cid) return children; 

		// if (this->cell_process.count(cid) == 0) return children;

		// check if given cell cannot have children
		int refinement_level = get_refinement_level(cid);
		if (refinement_level >= maximum_refinement_level) return children;



		children.reserve(8);

		indices_t indices = get_indices(cid);

		// get indices of next refinement level within this cell
		for (uint64_t
			z_shift = 0;
			z_shift < 2;
			z_shift++
		)
		for (uint64_t
			y_shift = 0;
			y_shift < 2;
			y_shift++
		)
		for (uint64_t
			x_shift = 0;
			x_shift < 2;
			x_shift++
		) {
      const indices_t index = 
      {{ 
         indices[0]*2 + x_shift,
         indices[1]*2 + y_shift,
         indices[2]*2 + z_shift
      }};

			children.push_back(
				get_cell_from_indices(index, refinement_level + 1)
			);
		}

		return children;
	}


  std::vector<uint64_t> get_siblings(const uint64_t cid) const
  {
    uint64_t cid_parent = get_parent(cid);
    std::vector<uint64_t> siblings = get_children(cid_parent);

    return siblings;
  }
  


  /*! \brief Is current cell at the tip of the graph tree?
   * Does only shallow search by considering one cell above.
   * This means that we implicitly assume connected graphs.
   *
   * TODO: make full search / or add option to do this
   */
  bool is_leaf(const uint64_t cid) const
  {

    // get cells directly above me 
    auto children = get_children(cid);
    for(auto cidc : children) if(data.count(cidc) > 0) return false;

    // as there is no children above, then this is a leaf
    return true;
  }



  /*
  std::vector<uint64_t> get_leafs(const uint64_t cid) const 
  {
		std::vector<uint64_t> children;

    // TODO implement
    int refinement_level = get_refinement_level(cid);
		if (refinement_level >= maximum_refinement_level) {
      children.push_back(cid);
      return children;
    }


  }
  */


  //-------------------------------------------------- 
  // Geometry

  indices_t get_size(int refinement_level) const
  {

    indices_t lens = 
    {{
        length[0] * (uint64_t(1) << refinement_level),
        length[1] * (uint64_t(1) << refinement_level),
        length[2] * (uint64_t(1) << refinement_level)
     }};


    return lens;
  }

  
  value_array_t mins;
  value_array_t maxs;
  
  void set_min(value_array_t& given_mins)
  {
    mins = given_mins;
  }

  void set_max(value_array_t& given_maxs)
  {
    maxs = given_maxs;
  }

  value_array_t get_min() const
  {
    return mins;
  }

  value_array_t get_max() const
  {
    return maxs;
  }


  value_array_t get_level_0_cell_length() const 
  {
    value_array_t 
      grid_start = get_min(),

      grid_stop  = get_max(),

      grid_length = 
      {{ 
         std::max(T(1), T(length[0]-1)),
         std::max(T(1), T(length[1]-1)),
         std::max(T(1), T(length[2]-1))
      }},

      ret = 
      {{
        (grid_stop[0] - grid_start[0]) / grid_length[0],
        (grid_stop[1] - grid_start[1]) / grid_length[1],
        (grid_stop[2] - grid_start[2]) / grid_length[2]
      }};

    return ret;
  }


  // get physical length of the box at refinement level
  value_array_t get_length(const int refinement_level) const {
    value_array_t length = get_level_0_cell_length();

    for(int rfl=1; rfl<=refinement_level; rfl++) {
      for(size_t i=0; i<D; i++)  length[i] /= T(2);
    }

    return length;
  }



  value_array_t get_center(
      const indices_t& index,
      const int refinement_level) const
  {

    const value_array_t error_val = {{
			std::numeric_limits<T>::quiet_NaN(),
			std::numeric_limits<T>::quiet_NaN(),
			std::numeric_limits<T>::quiet_NaN()
		}};

		if (refinement_level < 0
		|| refinement_level > maximum_refinement_level) {
			return error_val;
		}

		const uint64_t index_scaling_factor
			= uint64_t(1) << refinement_level;

		const indices_t max_index = {{
			length[0] * index_scaling_factor,
			length[1] * index_scaling_factor,
			length[2] * index_scaling_factor
		}};

		if ( index[0] > max_index[0]
			|| index[1] > max_index[1]
			|| index[2] > max_index[2]) {
			return error_val;
		}

		const T
			coordinate_scaling_factor  = 1.0 / T(index_scaling_factor),
			cell_offset_scaling_factor = 1.0 / T( uint64_t(1) << refinement_level) / 2;


		const value_array_t 
			grid_start          = get_min(),
      level_0_cell_length = get_level_0_cell_length(),
			ret_val = {{
				grid_start[0]
				+ T(index[0])
					* level_0_cell_length[0]
					* coordinate_scaling_factor
				+ level_0_cell_length[0]
					* cell_offset_scaling_factor,
				grid_start[1]
				+ T(index[1])
					* level_0_cell_length[1]
					* coordinate_scaling_factor
				+ level_0_cell_length[1]
					* cell_offset_scaling_factor,
				grid_start[2]
				+ T(index[2])
					* level_0_cell_length[2]
					* coordinate_scaling_factor
				+ level_0_cell_length[2]
					* cell_offset_scaling_factor
			}};

		return ret_val;

  }


  std::vector<uint64_t> get_cells(bool sorted=false) const
  {
    std::vector<uint64_t> all_cells;

    for (auto item: data) {
      uint64_t cid = item.first;

      all_cells.push_back( cid );
    };

    if (sorted && all_cells.size() > 0) {
      std::sort(all_cells.begin(), all_cells.end());
    }

    return all_cells;
  }


  //-------------------------------------------------- 
  // Adaptivity

  // clip every cell below threshold
  void clip_cells(const T threshold) {
    std::vector<uint64_t> below_threshold;
    T maxv = max_value();

    for(const uint64_t cid: get_cells(false)) {
      if( data.at(cid)/maxv < threshold ) below_threshold.push_back(cid);
    }

    for(const uint64_t cid: below_threshold) data.erase(cid);
  }



  // EXPERIMENTAL: THESE THINGS MIGHT NOT BE NEEDED AFTER ALL
  //
  // Cut parent cell that has children; only leaf cells should remain
  // void cut_roots() {
  //   auto all_cells = get_cells(true); // sorted=true
  //   // iterate in reverse, starting from the cell with highest refinement level
  //   // then collect all roots that can be cut
  //   // for(auto& cid : reverse(all_cells)) {
  //   //   std::cout << cid << "\n";
  //   // }
  // }


  /*! \brief split the given cell into 2^D children and remove the parent
   * NOTE: This is the safe and primary method to split cells as this 
   * cleans up parents.
   */

  // void split(const uint64_t cid) {

  //   auto children = get_children(cid);
  //   for(auto cidc : children) data[cidc] = T(0);

  //   data.erase(cid); 
  // }


  // // Update parent cell's value to be mean of its children cells' value
  // void update_from_children(const uint64_t cid)
  // {
  //   T mean = T(0);
  //   for(auto cidc : get_children(cid)) mean += get(cidc);
  //   data[cid] = mean / std::pow(2, D);
  // }


  // /// Recursive get from the tip of the tree
  // T get_from_leafs(const uint64_t cid) 
  // {
  //   if( is_leaf(cid) ) return data[cid]; 

  //   T mean = T(0);
  //   for(auto cidc : get_children(cid)) mean += get_from_leafs(cidc);
  //   return mean / std::pow(2, D);
  // }


  // void update_from_leafs(const uint64_t cid)
  // {
  //   data[cid] = get_from_leafs(cid);
  // }


  // get value from map. If it foes not exist, recursively check until parent is found
  T get_from_roots(uint64_t cid) const
  {
    int refinement_level = get_refinement_level(cid);
    indices_t ind = get_indices(cid);

    for(int rfl=refinement_level; rfl >= 0; rfl--) {
      cid = get_cell_from_indices(ind, rfl);

      auto it = data.find(cid);
      if( it != data.end() ) return it->second;

      for(size_t i=0; i<D; i++) {
        ind[i] /= 2;
      }
    }

    // we went through the tree and did not find any existing cell
    return T(0);
  }


  std::pair<T,int> get_value_and_level(uint64_t cid) const
  {
    int refinement_level = get_refinement_level(cid);
    indices_t ind = get_indices(cid);

    int lvl = 0;
    for(int rfl=refinement_level; rfl >= 0; rfl--) {
      cid = get_cell_from_indices(ind, rfl);

      auto it = data.find(cid);
      if( it != data.end() ) return std::make_pair(it->second, lvl);

      for(size_t i=0; i<D; i++) {
        ind[i] /= 2;
      }
      lvl++;
    }

    // we went through the tree and did not find any existing cell
    return std::make_pair(T(0), -1);
  }

  // Optimized? version of the above
  /*
  T get_from_roots_optimized(uint64_t cid) const
  {
    const_iterator it = data.find(cid);
    if (it == data.end()) {

      int refinement_level = get_refinement_level(cid);
      indices_t ind = get_indices(cid);

      for(int rfl=refinement_level-1; rfl >= 0; rfl--) {

        for(size_t i=0; i<D; i++) {
          ind[i] /= 2;
        }
        cid = get_cell_from_indices(ind, rfl);

        it = data.find(cid);
        if( it != data.end() ) return it->second;
      }

      // we went through the tree and did not find any existing cell
      return T(0);
    }

    return it->second;
  }
  */


  bool exists(const uint64_t cid) const
  {
    return data.count(cid) == data.end() ? false : true;
  }


  // update every value and divide until we hit the bottom
  void set_recursively(uint64_t cid, T val)
  {
    const int refinement_level = get_refinement_level(cid);

    T divisor = T(1);
    for(int rfl = refinement_level; rfl >= 0; rfl--){
      data[cid] += val/divisor;

      divisor *= std::pow(2.0, D);
    }
  }


  /*
    int refinement_level = get_refinement_level(cid);
    indices_t ind = get_indices(cid);

    for(int rfl=refinement_level; rfl >= 0; rfl--) {
      cid = get_cell_from_indices(ind, rfl);

      auto it = data.find(cid);
      if( it != data.end() ) return it->second;

      for(size_t i=0; i<D; i++) {
        ind[i] /= 2;
      }
    }
    */


  /// in-place addition
  inline AdaptiveMesh<T,D>& operator += (const AdaptiveMesh<T,D>& rhs)
  {
    return apply_elementwise(rhs, [](T a, T b) -> T { return a + b; } );
  }


  /// in-place subtraction
  inline AdaptiveMesh<T,D>& operator -= (const AdaptiveMesh<T,D>& rhs)
  {
    return apply_elementwise(rhs, [](T a, T b) -> T { return a - b; } );
  }


  /// General elementwise operator for applying fun(A_i, B_i) to mesh A and B 
  template<typename Lambda>
  inline AdaptiveMesh<T,D>& apply_elementwise(
      const AdaptiveMesh<T,D>& rhs,
      Lambda&& func)
  {

    // loop over sorted cells so that sizes of incoming cells is decreasing
    for(auto&& cid_rhs : rhs.get_cells(true)) {
      auto it = data.find(cid_rhs);
      T val_rhs = rhs.data.at(cid_rhs); // cell guaranteed to exists in rhs.data so we can use .at()

      if (it != data.end()) {
        it->second = func(it->second, val_rhs); // cell exists so we simply add
      } else {
        // tree creation branch

        int refinement_level = get_refinement_level(cid_rhs);
        if(refinement_level == 0) {
          T val = func(0, val_rhs);
          data.insert(std::make_pair(cid_rhs, val));
        } else {

          uint64_t cid_parent = rhs.get_parent(cid_rhs);

          T val_parent = T(0);
          const auto itp = rhs.data.find(cid_parent);
          if (itp != rhs.data.end()) val_parent = itp->second;

          T val_tree = T(0);
          const auto itt = data.find(cid_parent);
          if (itt != data.end()) val_tree = itt->second;

          T val_inv = func(val_tree, -val_parent); // inverse of func; gives old value of 
                                                   // cell before func was applied. NOTE: Works only for + and -
          T val = func(val_inv, val_rhs);

          data.insert( std::make_pair(cid_rhs, val) );
        }
      }
    }

    return *this;
  }

  /// get absolute maximum value in the grid
  // NOTE: lookup includes full mesh, not just leafs
  T max_value() const
  {
    T ret = T(0),
      val;
    for(auto cid : get_cells(true) ) {
      val = std::abs( data.at(cid) );
      ret = val > ret ? val : ret;
    };

    return ret;
  }



};






} // end of namespace vlasov