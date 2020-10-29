#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandPrintSopUnate(Abc_Frame_t* pAbc, int argc, char** argv);


void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_sopunate", Lsv_CommandPrintSopUnate, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};

struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;


///// for Nodes

void Lsv_NtkPrintNodes(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin));
    }
    if (Abc_NtkHasSop(pNtk)) {
      printf("The SOP of this node:\n%s", (char*)pObj->pData);
    }
  }
}

int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintNodes(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

//// for SopUnate


int Lsv_sortCompare(Abc_Obj_t ** a, Abc_Obj_t ** b) {
  return Abc_ObjId(*a) > Abc_ObjId(*b);
}


void printUnate(const char * str, Vec_Ptr_t * vec) {
  Abc_Obj_t * pEntry;
  int i, sz = Vec_PtrSize(vec);

  if (sz == 0) ;
  else{
  printf("%s: ", str);
  Vec_PtrForEachEntry( Abc_Obj_t *, vec, pEntry, i )
    printf("%s%c", Abc_ObjName(pEntry), i != sz-1? ',':'\n');
    }
}



void Lsv_NtkPrintSopUnate(Abc_Ntk_t* pNtk) {
  Abc_Obj_t * pObj;
  int i, j;

  Abc_NtkForEachNode(pNtk, pObj, i) {
    char * pSop = (char *)pObj->pData;
    char * pCube;
    char val;
    int nFanins = Abc_ObjFaninNum(pObj);
    bool unateTable[nFanins][2] = {};
    Abc_SopForEachCube(pSop, nFanins, pCube) {
      bool isOnset = *(pCube + nFanins + 1) - '0';
      Abc_CubeForEachVar(pCube, val, j) {
        if (val == '-') continue;
        bool isOne = val - '0';
        unateTable[j][isOne == isOnset] = 1;
      }
    }
    
    Vec_Ptr_t * posUnateVec = Vec_PtrAlloc( nFanins );
    Vec_Ptr_t * negUnateVec = Vec_PtrAlloc( nFanins );
    Vec_Ptr_t * binateVec = Vec_PtrAlloc( nFanins );
    Abc_Obj_t * pFanin;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      if (unateTable[j][0] && unateTable[j][1]) 
        Vec_PtrPush( binateVec, pFanin );
      else {
        if (!unateTable[j][0]) Vec_PtrPush( posUnateVec, pFanin );
        if (!unateTable[j][1]) Vec_PtrPush( negUnateVec, pFanin );
      }
    }

    Vec_PtrSort( posUnateVec, (int (*)()) Lsv_sortCompare );
    Vec_PtrSort( negUnateVec, (int (*)()) Lsv_sortCompare );
    Vec_PtrSort( binateVec, (int (*)()) Lsv_sortCompare );
    
    
    int sz = Vec_PtrSize(posUnateVec)+Vec_PtrSize(negUnateVec)+Vec_PtrSize(binateVec);

    if (sz == 0) ;
    else {
    printf("node %s:\n", Abc_ObjName(pObj));
    printUnate( "+unate inputs", posUnateVec );
    printUnate( "-unate inputs", negUnateVec );
    printUnate( "binate inputs", binateVec );
}
    Vec_PtrFree( posUnateVec );
    Vec_PtrFree( negUnateVec );
    Vec_PtrFree( binateVec );
  }
}



int Lsv_CommandPrintSopUnate(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintSopUnate(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_sopunate [-h]\n");
  Abc_Print(-2, "\t        prints the unate information for each node\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}