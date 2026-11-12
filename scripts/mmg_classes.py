from writeAPI import *

MMG5_int = "c_int"

def setAPIClasses(api : pythonAPI):

    # Parameters

    par = mmgClass("MMG5_Par")
    par.addArgs([arg("hmin","c_double")])
    par.addArgs([arg("hmax","c_double")])
    par.addArgs([arg("hausd","c_double")])
    par.addArgs([arg("ref",MMG5_int)])
    par.addArgs([arg("elt","c_int8")])

    MMG5_pPar = "POINTER("+par.name+")"
    api.addType("pPar",MMG5_pPar)
    api.classes.append(par)

    # Point

    point = mmgClass("MMG5_Point")
    point.addArgs([arg("c","c_double * 3")])
    point.addArgs([arg("n","c_double * 3")])
    #point.addArgs([arg("src","c_double")])
    point.addArgs([arg("ref",MMG5_int)])
    point.addArgs([arg("xp",MMG5_int)])
    point.addArgs([arg("tmp",MMG5_int)])
    point.addArgs([arg("flag",MMG5_int)])
    point.addArgs([arg("s",MMG5_int)])
    point.addArgs([arg("tag","c_uint16")])
    point.addArgs([arg("tagdel","c_int8")])

    MMG5_pPoint = "POINTER("+point.name+")"
    api.addType("pPoint",MMG5_pPoint)
    api.classes.append(point)

    # xPoint

    xpoint = mmgClass("MMG5_xPoint")
    xpoint.addArgs([arg("n1","c_double * 3")])
    xpoint.addArgs([arg("n2","c_double * 3")])
    xpoint.addArgs([arg("nnor","c_int8")])

    MMG5_pxPoint = "POINTER("+xpoint.name+")"
    api.addType("pxPoint",MMG5_pxPoint)
    api.classes.append(xpoint)

    # Edge

    edge = mmgClass("MMG5_Edge")
    edge.addArgs([arg("a",MMG5_int),arg("b",MMG5_int)])
    edge.addArgs([arg("ref",MMG5_int),arg("base",MMG5_int)])
    edge.addArgs([arg("tag","c_uint16")])

    MMG5_pEdge = "POINTER("+edge.name+")"
    api.addType("pEdge",MMG5_pEdge)
    api.classes.append(edge)

    # Tria

    tria = mmgClass("MMG5_Tria")
    tria.addArgs([arg("qual","c_double")])
    tria.addArgs([arg("v",MMG5_int+"*3")])
    tria.addArgs([arg("ref",MMG5_int)])
    tria.addArgs([arg("base",MMG5_int)])
    tria.addArgs([arg("cc",MMG5_int)])
    tria.addArgs([arg("edg",MMG5_int+"*3")])
    tria.addArgs([arg("flag",MMG5_int)])
    tria.addArgs([arg("tag","c_uint16 * 3")])

    MMG5_pTria = "POINTER("+tria.name+")"
    api.addType("pTria",MMG5_pTria)
    api.classes.append(tria)

    # Quad

    quad = mmgClass("MMG5_Quad")
    quad.addArgs([arg("v",MMG5_int+"*4")])
    quad.addArgs([arg("ref",MMG5_int)])
    quad.addArgs([arg("base",MMG5_int)])
    quad.addArgs([arg("edg",MMG5_int+"*4")])
    quad.addArgs([arg("tag","c_uint16 * 4")])

    MMG5_pQuad = "POINTER("+quad.name+")"
    api.addType("pQuad",MMG5_pQuad)
    api.classes.append(quad)

    # Tetra

    tetra = mmgClass("MMG5_Tetra")
    tetra.addArgs([arg("qual","c_double")])
    tetra.addArgs([arg("v",MMG5_int)])
    tetra.addArgs([arg("ref",MMG5_int)])
    tetra.addArgs([arg("base",MMG5_int)])
    tetra.addArgs([arg("mark",MMG5_int)])
    tetra.addArgs([arg("xt",MMG5_int)])
    tetra.addArgs([arg("flag",MMG5_int)])
    tetra.addArgs([arg("tag","c_uint16")])

    MMG5_pTetra = "POINTER("+tetra.name+")"
    api.addType("pTetra",MMG5_pTetra)
    api.classes.append(tetra)

    # xTetra

    xtetra = mmgClass("MMG5_xTetra")
    xtetra.addArgs([arg("ref",MMG5_int+"*4")])
    xtetra.addArgs([arg("edg",MMG5_int+"*6")])
    xtetra.addArgs([arg("ftag","c_uint16 * 4")])
    xtetra.addArgs([arg("tag","c_uint16 * 6")])
    xtetra.addArgs([arg("ori","c_int8")])

    MMG5_pxTetra = "POINTER("+xtetra.name+")"
    api.addType("pxTetra",MMG5_pxTetra)
    api.classes.append(xtetra)

    # Prism

    prism = mmgClass("MMG5_Prism")
    prism.addArgs([arg("v",MMG5_int+"*6")])
    prism.addArgs([arg("ref",MMG5_int)])
    prism.addArgs([arg("base",MMG5_int)])
    prism.addArgs([arg("flag",MMG5_int)])
    prism.addArgs([arg("xpr",MMG5_int)])
    prism.addArgs([arg("tag","c_int8")])

    MMG5_pPrism = "POINTER("+prism.name+")"
    api.addType("pPrism",MMG5_pPrism)
    api.classes.append(prism)

    # xPrism

    xprism = mmgClass("MMG5_xPrism")
    xprism.addArgs([arg("ref",MMG5_int+"*5")])
    xprism.addArgs([arg("edg",MMG5_int+"*9")])
    xprism.addArgs([arg("ftag","c_uint16 * 5")])
    xprism.addArgs([arg("tag","c_uint16 * 9")])

    MMG5_pxPrism = "POINTER("+xprism.name+")"
    api.addType("pxPrism",MMG5_pxPrism)
    api.classes.append(xprism)

    # Mat

    mat = mmgClass("MMG5_Mat")
    mat.addArgs([arg("dospl","c_int8")])
    mat.addArgs([arg("ref",MMG5_int)])
    mat.addArgs([arg("rin",MMG5_int)])
    mat.addArgs([arg("rex",MMG5_int)])

    MMG5_pMat = "POINTER("+mat.name+")"
    api.addType("pMat",MMG5_pMat)
    api.classes.append(mat)

    # InvMat

    invmat = mmgClass("MMG5_InvMat")
    invmat.addArgs([arg("offset",MMG5_int)])
    invmat.addArgs([arg("size",MMG5_int)])
    invmat.addArgs([arg("lookup","POINTER(c_int)")])

    MMG5_pInvMat = "POINTER("+invmat.name+")"
    api.addType("pInvMat",MMG5_pInvMat)
    api.classes.append(invmat)

    # Info

    info = mmgClass("MMG5_Info")
    info.addArgs([arg("par",MMG5_pPar)])
    info.addArgs([arg("dhd","c_double"),arg("hmin","c_double"),arg("hmax","c_double")])
    info.addArgs([arg("hsiz","c_double"),arg("hgrad","c_double"),arg("hgradreq","c_double")])
    info.addArgs([arg("hausd","c_double")])
    info.addArgs([arg("min","c_double * 3"),arg("max","c_double * 3")])
    info.addArgs([arg("delta","c_double"),arg("ls","c_double")])
    info.addArgs([arg("lxreg","c_double"),arg("rmc","c_double")])
    info.addArgs([arg("br","POINTER(c_int)")])
    info.addArgs([arg("isoref",MMG5_int),arg("nsd",MMG5_int)])
    info.addArgs([arg("mem","c_int"),arg("npar","c_int"),arg("npari","c_int")])
    info.addArgs([arg("nbr","c_int"),arg("nbri","c_int")])
    info.addArgs([arg("opnbdy","c_int"),arg("renum","c_int")])
    info.addArgs([arg("PROctree","c_int"),arg("nmati","c_int")])
    info.addArgs([arg("nmat","c_int"),arg("imprim","c_int")])
    info.addArgs([arg("nreg","c_int8"),arg("xreg","c_int8")])
    info.addArgs([arg("ddebug","c_int8"),arg("badkal","c_int8")])
    info.addArgs([arg("iso","c_int8"),arg("isosurf","c_int8")])
    info.addArgs([arg("setfem","c_int8"),arg("fem","c_int8")])
    info.addArgs([arg("lag","c_int8"),arg("parTyp","c_int8")])
    info.addArgs([arg("sethmin","c_int8"),arg("sethmax","c_int8")])
    info.addArgs([arg("ani","c_uint8"),arg("optim","c_uint8")])
    info.addArgs([arg("optimLES","c_uint8"),arg("noinsert","c_uint8")])
    info.addArgs([arg("noswap","c_uint8"),arg("nomove","c_uint8")])
    info.addArgs([arg("nosurf","c_uint8"),arg("nosizereq","c_uint8")])
    info.addArgs([arg("metRidTyp","c_uint8")])
    info.addArgs([arg("fparam","c_char_p")])
    info.addArgs([arg("mat",MMG5_pMat)])
    info.addArgs([arg("invmat",invmat.name)])

    api.classes.append(info)

    # hgeom

    hgeom = mmgClass("MMG5_hgeom")
    hgeom.addArgs([arg("a",MMG5_int),arg("b",MMG5_int)])
    hgeom.addArgs([arg("ref",MMG5_int),arg("nxt",MMG5_int)])
    hgeom.addArgs([arg("tag","c_uint16")])

    api.classes.append(hgeom)

    # HGeom

    HGeom = mmgClass("MMG5_HGeom")
    HGeom.addArgs([arg("geom","POINTER("+hgeom.name+")")])
    HGeom.addArgs([arg("siz",MMG5_int),arg("max",MMG5_int),arg("nxt",MMG5_int)])

    api.classes.append(HGeom)

    # hedge

    hedge = mmgClass("MMG5_hedge")
    hedge.addArgs([arg("a",MMG5_int),arg("b",MMG5_int),arg("nxt",MMG5_int)])
    hedge.addArgs([arg("k",MMG5_int),arg("s",MMG5_int)])

    api.classes.append(hedge)

    # hash

    Hash = mmgClass("MMG5_Hash")
    Hash.addArgs([arg("siz",MMG5_int),arg("max",MMG5_int),arg("nxt",MMG5_int)])
    Hash.addArgs([arg("item","POINTER("+hedge.name+")")])

    api.classes.append(Hash)

    # mesh

    mesh = mmgClass("MMG5_Mesh")
    mesh.addArgs([arg("memMax","c_size_t")])
    mesh.addArgs([arg("memCur","c_size_t")])
    mesh.addArgs([arg("gap","c_double")])
    mesh.addArgs([arg("ver","c_int")])
    mesh.addArgs([arg("dim","c_int")])
    mesh.addArgs([arg("type","c_int")])
    mesh.addArgs([arg("npi",MMG5_int),arg("nti",MMG5_int),arg("nai",MMG5_int),arg("nei",MMG5_int)])
    mesh.addArgs([arg("np",MMG5_int),arg("na",MMG5_int),arg("nt",MMG5_int),arg("ne",MMG5_int)])
    mesh.addArgs([arg("npmax",MMG5_int),arg("namax",MMG5_int),arg("ntmax",MMG5_int),arg("nemax",MMG5_int)])
    mesh.addArgs([arg("xpmax",MMG5_int),arg("xtmax",MMG5_int)])
    mesh.addArgs([arg("nquad",MMG5_int),arg("nprism",MMG5_int)])
    mesh.addArgs([arg("nsols","c_int")])
    mesh.addArgs([arg("nc1",MMG5_int)])
    mesh.addArgs([arg("base",MMG5_int)])
    mesh.addArgs([arg("mark",MMG5_int)])
    mesh.addArgs([arg("xp",MMG5_int),arg("xt",MMG5_int),arg("xpr",MMG5_int)])
    mesh.addArgs([arg("npnil",MMG5_int)])
    mesh.addArgs([arg("nenil",MMG5_int)])
    mesh.addArgs([arg("nanil",MMG5_int)])
    mesh.addArgs([arg("adja","POINTER("+MMG5_int+")")])
    mesh.addArgs([arg("adjt","POINTER("+MMG5_int+")")])
    mesh.addArgs([arg("adjapr","POINTER("+MMG5_int+")")])
    mesh.addArgs([arg("adjq","POINTER("+MMG5_int+")")])
    mesh.addArgs([arg("ipar","POINTER(c_int)")])
    mesh.addArgs([arg("point",MMG5_pPoint)])
    mesh.addArgs([arg("xpoint",MMG5_pxPoint)])
    mesh.addArgs([arg("tetra",MMG5_pTetra)])
    mesh.addArgs([arg("xtetra",MMG5_pxTetra)])
    mesh.addArgs([arg("prism",MMG5_pPrism)])
    mesh.addArgs([arg("xprism",MMG5_pxPrism)])
    mesh.addArgs([arg("tria",MMG5_pTria)])
    mesh.addArgs([arg("quadra",MMG5_pQuad)])
    mesh.addArgs([arg("edge",MMG5_pEdge)])
    mesh.addArgs([arg("htab",HGeom.name)])
    mesh.addArgs([arg("info",info.name)])
    mesh.addArgs([arg("namein","c_char_p")])
    mesh.addArgs([arg("nameout","c_char_p")])

    MMG5_pMesh = "POINTER("+mesh.name+")"
    api.addType("pMesh",MMG5_pMesh)
    api.classes.append(mesh)

    # sol 

    sol = mmgClass("MMG5_Sol")

    sol.addArgs([arg("ver","c_int")])
    sol.addArgs([arg("dim","c_int")])
    sol.addArgs([arg("np",MMG5_int)])
    sol.addArgs([arg("npmax",MMG5_int)])
    sol.addArgs([arg("npi",MMG5_int)])
    sol.addArgs([arg("size","c_int")])
    sol.addArgs([arg("type","c_int")])
    sol.addArgs([arg("entities","c_int")])
    sol.addArgs([arg("m","POINTER(c_double)")])
    sol.addArgs([arg("umin","c_double")])
    sol.addArgs([arg("umax","c_double")])
    sol.addArgs([arg("namein","c_char_p")])
    sol.addArgs([arg("nameout","c_char_p")])

    sol.addConstruct(["size",1])
    MMG5_pSol = "POINTER("+sol.name+")"
    api.addType("pSol",MMG5_pSol)
    api.classes.append(sol)

