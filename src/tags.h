/*
============================================================================
DELLY: Structural variant discovery by integrated PE mapping and SR analysis
============================================================================
Copyright (C) 2012 Tobias Rausch

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
============================================================================
Contact: Tobias Rausch (rausch@embl.de)
============================================================================
*/

#ifndef TAGS_H
#define TAGS_H

namespace torali {

  // Constants
  #define MAX_CHROM_SIZE 250000000


  // Tags
  struct DeletionTag;
  struct DuplicationTag;
  struct InversionTag;
  struct TranslocationTag;

  template<typename SvTag>
    struct SVType {
    };


  struct RedundancyFilterTag;
  struct NoRedundancyFilterTag;
  
  template<typename CoverageTag>
    struct CoverageType {
    };

  // F+ 0
  // F- 1
  // R+ 2
  // R- 3

  template<typename TBamRecord>
  inline int
    getStrandIndependentOrientation(TBamRecord const& al) {
    if (al.AlignmentFlag & 0x0040) {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position < al.MatePosition) ? 0 : 1;
	else return (al.Position < al.MatePosition) ? 2 : 3;
      } else {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position > al.MatePosition) ? 2 : 3;
	else return (al.Position > al.MatePosition) ? 0 : 1;
      }
    } else {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position < al.MatePosition) ? 1 : 0;
	else return (al.Position < al.MatePosition) ? 2 : 3;
      } else {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position > al.MatePosition) ? 2 : 3;
	else return (al.Position > al.MatePosition) ? 1 : 0;
      }
    }
  }

  // Deletions
  template<typename TBamRecord>
    inline int 
    _getSpanOrientation(TBamRecord const&, int const, SVType<DeletionTag>) {
    return 1;
  }

  // Duplications
  template<typename TBamRecord>
    inline int 
    _getSpanOrientation(TBamRecord const&, int const, SVType<DuplicationTag>) {
    return 1;
  }

  // Left- or right-spanning
  template<typename TBamRecord>
    inline int 
    _getSpanOrientation(TBamRecord const& al, int const defaultOrient, SVType<InversionTag>) {
    int orient = getStrandIndependentOrientation(al);
    if (al.AlignmentFlag & 0x0040) {
      if (defaultOrient == 0) {
	if (((orient==2) && (al.Position < al.MatePosition)) || ((orient == 3) && (al.Position > al.MatePosition))) return 1;
      } else if (defaultOrient == 1) {
	if (((orient==2) && (al.Position > al.MatePosition)) || ((orient == 3) && (al.Position < al.MatePosition))) return 1;
      } else if (defaultOrient == 2) {
	if (((orient==0) && (al.Position < al.MatePosition)) || ((orient == 1) && (al.Position > al.MatePosition))) return 1;
      } else if (defaultOrient == 3) {
	if (((orient==0) && (al.Position > al.MatePosition)) || ((orient == 1) && (al.Position < al.MatePosition))) return 1;
      }
      return 0;
    } else {
      if (defaultOrient == 0) {
	if (((orient==2) && (al.Position > al.MatePosition)) || ((orient == 3) && (al.Position < al.MatePosition))) return 1;
      } else if (defaultOrient == 1) {
	if (((orient==2) && (al.Position < al.MatePosition)) || ((orient == 3) && (al.Position > al.MatePosition))) return 1;
      } else if (defaultOrient == 2) {
	if (((orient==0) && (al.Position > al.MatePosition)) || ((orient == 1) && (al.Position < al.MatePosition))) return 1;
      } else if (defaultOrient == 3) {
	if (((orient==0) && (al.Position < al.MatePosition)) || ((orient == 1) && (al.Position > al.MatePosition))) return 1;
      }
      return 0;
    }
  }




  template<typename TBamRecord>
    inline int 
    _inOrderAssign(TBamRecord const& al, bool flipped) {
    if (!flipped) {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) {
	  return (al.AlignmentFlag & 0x0040) ? 0 : 1;
	} else {
	  return 2;
	}
      } else {
	if (!(al.AlignmentFlag & 0x0020)) {
	  return 3;
	} else {
	  return (al.AlignmentFlag & 0x0040) ? 1 : 0;
	}
      }
    } else {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) {
	  return 2;
	} else {
	  return (al.AlignmentFlag & 0x0040) ? 0 : 1;
	}
      } else {
	if (!(al.AlignmentFlag & 0x0020)) {
	  return (al.AlignmentFlag & 0x0040) ? 1 : 0;
	} else {
	  return 3;
	}
      }
    }
  }


  template<typename TBamRecord>
    inline int 
    _getSpanOrientation(TBamRecord const& al, int const defaultOrient, SVType<TranslocationTag>) {
    int orient = getStrandIndependentOrientation(al);
    bool flipped = ( ((defaultOrient<2) && (orient>=2)) || ((defaultOrient>=2) && (orient<2)) );
    bool inOrder = (_inOrderAssign(al, flipped) == defaultOrient);
    if (flipped) {
      if (inOrder) return 0;
      else return 1;
    } else {
      if (inOrder) return 2;
      else return 3;
    }
  }

  // Unique paired-end data structure for single chromosome only
  struct Hit {
    int32_t minPos;
    int32_t maxPos;
  
    template<typename TBamRecord>
  Hit(TBamRecord const& al) : minPos(std::min(al.Position, al.MatePosition)), maxPos(std::max(al.Position, al.MatePosition)) {}

    bool operator <(Hit const& other) const {
      return ((minPos<other.minPos) || ((minPos==other.minPos) && (maxPos<other.maxPos)));
    }
  };

  // Structural variant record
  struct StructuralVariantRecord {
    int svStartBeg;
    int svStartEnd;
    int svEndBeg;
    int svEndEnd;
    int svStart;
    int svEnd;
    int peSupport;
    int srSupport;
    int wiggle;
    double srAlignQuality;
    unsigned int id;
    bool precise;
    int ct;
    uint16_t peMapQuality;
    int32_t chr;
    int32_t chr2;
    std::string consensus;

    StructuralVariantRecord() {}
  StructuralVariantRecord(int32_t const c, int const s, int const e) : svStart(s), svEnd(e), chr(c) {}
  };

  template<typename TSV>
  struct SortSVs : public std::binary_function<TSV, TSV, bool>
  {
    inline bool operator()(TSV const& sv1, TSV const& sv2) {
      return ((sv1.chr<sv2.chr) || ((sv1.chr==sv2.chr) && (sv1.svStart<sv2.svStart)) || ((sv1.chr==sv2.chr) && (sv1.svStart==sv2.svStart) && (sv1.svEnd<sv2.svEnd)));
    }
  };



  // SV Paired-end checks

  // Deletions, duplications and inversions
  template<typename TPos, typename TTag>
    inline TPos
    _minCoord(TPos const position, TPos const matePosition, SVType<TTag>) {
    return std::min(position, matePosition);
  }

  // Translocations
  template<typename TPos>
    inline TPos
    _minCoord(TPos const position, TPos const, SVType<TranslocationTag>) {
    return position;
  }

  // Deletions, duplications and inversions
  template<typename TPos, typename TTag>
    inline TPos
    _maxCoord(TPos const position, TPos const matePosition, SVType<TTag>) {
    return std::max(position, matePosition);
  }

  // Translocations
  template<typename TPos>
    inline TPos
    _maxCoord(TPos const, TPos const matePosition, SVType<TranslocationTag>) {
    return matePosition;
  }

  // Deletions, duplications and inversions
  template<typename TQualities, typename TTag>
    inline void
    _resetQualities(TQualities& qualities, SVType<TTag>) {
    qualities.clear();
  }

  // Translocations
  template<typename TQualities>
    inline void
    _resetQualities(TQualities&, SVType<TranslocationTag>) {
    // Nop
  }

  // Deletions
  template<typename TRef, typename TPos>
    inline bool
    _mappingPos(TRef const refID, TRef const mateRefID, TPos const position, TPos const matePosition, SVType<DeletionTag>) {
    return ((refID!=mateRefID) || (position==matePosition));
  }

  // Duplications
  template<typename TRef, typename TPos>
    inline bool
    _mappingPos(TRef const refID, TRef const mateRefID, TPos const position, TPos const matePosition, SVType<DuplicationTag>) {
    return ((refID!=mateRefID) || (std::abs(position - matePosition) < 100 ));
  }

  // Inversions
  template<typename TRef, typename TPos>
    inline bool
    _mappingPos(TRef const refID, TRef const mateRefID, TPos const position, TPos const matePosition, SVType<InversionTag>) {
    return ((refID!=mateRefID) || (position==matePosition));
  }

  // Translocations
  template<typename TRef, typename TPos>
    inline bool
    _mappingPos(TRef const refID, TRef const mateRefID, TPos const, TPos const, SVType<TranslocationTag>) {
    return (refID==mateRefID);
  }

  // Deletions, duplications and inversions
  template<typename TRef, typename TPos, typename TTag>
    inline bool
    _mappingPosGeno(TRef const refID, TRef const mateRefID, TPos const position, TPos const matePosition, SVType<TTag> svType) {
    return _mappingPos(refID, mateRefID, position, matePosition, svType);
  }

  // Translocations
  template<typename TRef, typename TPos>
    inline bool
    _mappingPosGeno(TRef const refID, TRef const mateRefID, TPos const position, TPos const matePosition, SVType<TranslocationTag>) {
    return ((refID==mateRefID) && (position==matePosition));
  }

  // Deletions, duplications and inversions
  template<typename TRef, typename TPos, typename TTag>
    inline bool
    _firstPairObs(TRef const, TRef const, TPos const position, TPos const matePosition, SVType<TTag>) {
    return (position<matePosition);
  }

  // Translocations
  template<typename TRef, typename TPos>
    inline bool
    _firstPairObs(TRef const refID, TRef const mateRefID, TPos const, TPos const, SVType<TranslocationTag>) {
    return (refID<mateRefID);
  }

  // Deletions
  template<typename TISize>
    inline bool
    _acceptedInsertSize(TISize const maxNormalISize, TISize const, TISize const iSize, SVType<DeletionTag>) {
    return (maxNormalISize > iSize);
  }

  // Duplications
  template<typename TISize>
    inline bool
    _acceptedInsertSize(TISize const, TISize const median, TISize const iSize, SVType<DuplicationTag>) {
    // Exclude the chimeras in mate-pair libraries
    return !((median<1000) || ((median>=1000) && (iSize >=1000)));
  }

  // Other SV Types
  template<typename TISize, typename TTag>
    inline bool
    _acceptedInsertSize(TISize const, TISize const, TISize const, SVType<TTag>) {
    return false;
  }

  // Deletions
  template<typename TOrientation>
  inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<DeletionTag>) {
    return (def != lib);
  }

  // Duplications
  template<typename TOrientation>
    inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<DuplicationTag>) {
    if (def==0) return (lib != 1);
    else if (def==1) return (lib!=0);
    else if (def==2) return (lib!=3);
    else if (def==3) return (lib!=2);
    else return true;
  }

  // Inversion
  template<typename TOrientation>
    inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<InversionTag>) {
    return (((def<=1) && (lib!=2) && (lib!=3)) || ((def>=2) && (lib!=0) && (lib!=1)));
  }

  // Other SV Types
  template<typename TOrientation, typename TTag>
    inline bool
    _acceptedOrientation(TOrientation const, TOrientation const, SVType<TTag>) {
    return false;
  }


  // Deletions
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, int const, int const, SVType<DeletionTag>) {
    //std::cout << pair1Min << ',' << pair1Max << ',' << pair1ReadLength << ',' << pair1maxNormalISize << ',' << pair2Min << ',' << pair2Max << ',' << pair2ReadLength << ',' << pair2maxNormalISize << std::endl;
    if ((pair2Min + pair2ReadLength - pair1Min) > pair1maxNormalISize) return true;
    if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize)) return true;
    if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize)) return true;
    return false;
  }

  // Duplications
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, int const, int const, SVType<DuplicationTag>) {
    if ((pair2Min + pair2ReadLength - pair1Min) > pair2maxNormalISize) return true;
    if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize)) return true;
    if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize)) return true;
    return false;
  }


  // Inversions
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, int const ct1, int const ct2, SVType<InversionTag>) {
    // Do both pairs support the same inversion type (left- or right-spanning)
    if (ct1 != ct2) return true;
    if (ct1==1) {
      // Left-spanning inversions
      if ((pair2Min + pair2ReadLength - pair1Min) > pair1maxNormalISize) return true;
      if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize)) return true;
      if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize)) return true;
    } else {
      // Right-spanning inversions
      if ((pair2Min + pair2ReadLength - pair1Min) > pair2maxNormalISize) return true;
      if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize)) return true;
      if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize)) return true;
    }
    return false;
  }


  // Translocations
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, int const ct1, int const ct2, SVType<TranslocationTag>) {
    // Do both pairs support the same translocation type
    if (ct1 != ct2) return true;

    // Check read offsets
    if (ct1%2==0) {
      if ((pair2Min + pair2ReadLength - pair1Min) > pair1maxNormalISize) return true;
      if (ct1>=2) {
	if (pair2Max < pair1Max) {
	  if ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize) return true;
	} else {
	  if ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize) return true;
	}
      } else {
	if (pair2Max < pair1Max) {
	  if ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize) return true;
	} else {
	  if ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize) return true;
	}
      }
    } else {
      if ((pair2Min + pair2ReadLength - pair1Min) > pair2maxNormalISize) return true;
      if (ct1>=2) {
	if (pair2Max < pair1Max) {
	  if ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize) return true;
	} else {
	  if ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize) return true;
	}
      } else {
	if (pair2Max < pair1Max) {
	  if ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize) return true;
	} else {
	  if ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize) return true;
	}
      }
    }
    return false;
  }


}

#endif
