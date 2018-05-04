## reference face coordinates (for a cube)
#v -0.018826 0.018826 -0.981174
#v -0.018826 0.018826 0.981174
#v -0.159622 0.018826 0.981174
#v -0.159622 0.018826 -0.981174
import sys
import math

Acoords = []
AcoordsOrdered = []
Atcoords = []
AtcoordsOrdered = []
Anormals = []
Afaces = []

vcam = [1, 1, 1]
mname = 'tbuilding'

def vCross(a, b):
    return [
            a[1]*b[2] - a[2]*b[1],
            a[2]*b[0] - a[0]*b[2],
            a[0]*b[1] - a[1]*b[0]
            ]

while 1:
  l = sys.stdin.readline()
  if len(l) <= 0:
    break

  tok = l.split(' ')

  if(tok[0] == 'v'):
    i = int(1)
    while(len(tok) > i):
      Acoords += [float(tok[i])]
      i += 1

  elif(tok[0] == 'vn'):
    print('//vn='+str([float(tok[1]), float(tok[2]), float(tok[3])]))
    Anormals += [float(tok[1]), float(tok[2]), float(tok[3])]

  elif(tok[0] == 'vt'):
    Atcoords += [float(tok[1])] ## U
    Atcoords += [float(tok[2])] ## V
    #W = [float(tok[3])] ## W (ignored)

  elif(tok[0] == 'f'):
    triangles = int(1)

    if len(tok) == 5:
      triangles = int(2)
      tokF = [[tok[0], tok[3], tok[4], tok[1]], [tok[0], tok[1], tok[2], tok[3]]]
    else:
      tokF = [[tok[0], tok[1], tok[2], tok[3]]]

    print('//tokf='+str(tokF))

    AcoordsOffset = len(AcoordsOrdered)/3

    while(triangles > 0):

      i = int(1)
      face = []
      vnormal = []
      tokT = tokF[triangles-1]

      while(len(tokT) > i):
        tokV = tokT[i].split('/')

        if tokV[0] != '':
          face += [int(tokV[0])-1]
        else:
          face += [int(0)]

        if tokV[1] != '':
          idx = int(tokV[1])-1
          AtcoordsOrdered += [Atcoords[idx*2], Atcoords[idx*2+1]]
        else:
          AtcoordsOrdered += [0, 0]

        if tokV[2] != '':
          idx = (int(tokV[2])-1) * 3
          vnormal = [Anormals[idx], Anormals[idx+1], Anormals[idx+2]]
          print('// vnormal='+str(vnormal)+' (idx='+str(idx)+')')
          
        i += 1
      ##-- end a/b/c token parsing

      u = [
              (Acoords[face[1]*3] - Acoords[face[0]*3]) * vcam[0],
              (Acoords[face[1]*3+1] - Acoords[face[0]*3+1]) * vcam[1],
              (Acoords[face[1]*3+2] - Acoords[face[0]*3+2]) * vcam[2]
          ]
      v = [
              (Acoords[face[2]*3] - Acoords[face[0]*3]) * vcam[0],
              (Acoords[face[2]*3+1] - Acoords[face[0]*3+1]) * vcam[1],
              (Acoords[face[2]*3+2] - Acoords[face[0]*3+2]) * vcam[2]
          ]
      uvN = vCross(u, v)
      vuN = vCross(v, u)

      print('// u='+str(u))
      print('// v='+str(v))
      print('// uvN='+str(uvN[0])+str(uvN[1]) + str(uvN[2]))

      nA = vnormal

      print ('// normal=' + str(nA[0]) + ' ' + str(nA[1]) + ' ' + str(nA[2]))

      ## face U, V cross product coliniear with normal?

      uvD = nA[0]*uvN[0] + nA[1]*uvN[1] + nA[2]*uvN[2]
      vuD = nA[0]*vuN[0] + nA[1]*vuN[1] + nA[2]*vuN[2]
    
      AfacesOffset = len(Afaces)
      Afaces += [face[0]]

      if uvD < vuD:
          Afaces += [face[2], face[1]]
      else:
          Afaces += [face[1], face[2]]

      flen = len(Afaces)
      #for i in [1,2,3]:
      #    Afaces[flen-i] += AcoordsOffset

      for i in [0,1,2]:
          for d in [0,1,2]:
              AcoordsOrdered += [Acoords[(Afaces[AfacesOffset+i]*3)+d]]

      for i in [1,2,3]:
          Afaces[flen-i] = flen-i

      triangles -= 1
    ##-- end triangles

    ## calculate face normal
    ## approx unitvec from origin to face midpoint (only works for convex models)
    #nA = [Acoords[face[0]*3], Acoords[face[0]*3+1], Acoords[face[0]*3+2]]
    #for k in [1, 2]:
    #    for d in [0, 1, 2]:
    #        nA[d] += ((Acoords[face[k]*3+d] - Acoords[face[k-1]*3+d]) * 0.5)
    #nA = [nA[0], nA[1], nA[2]]


    #if uvN[0]*nA[0] + uvN[1]*nA[1] + uvN[2]*nA[2] < 0:
    #    print "// ->"
    #    Afaces += [face[1], face[2], face[0]]
    #else:
    #    print "// <-"
    #    Afaces += [face[2], face[1], face[0]]

  else:
      print ('// ignoring line ' + tok[0])

## print model coordinates
print ('static model_coord_t model_' + mname + '_coords[] = {')
idx = 0
c = AcoordsOrdered
while idx < len(c):
  eol = ',' if idx+3 < len(c) else ''
  print (str(c[idx]) + ', ' + str(c[idx+1]) + ', ' + str(c[idx+2]) + eol)
  idx += 3
print ('};')

## print model texture coordinates
print ('static model_texcoord_t model_' + mname + '_texcoords[] = {')
idx = 0
c = AtcoordsOrdered
while idx < len(c):
  eol = ',' if idx+2 < len(c) else ''
  print (str(c[idx]) + ', ' + str(c[idx+1]) + eol)
  idx += 2
print ('};')

## print model face indices
print ('static model_index_t model_' + mname + '_indices[] = {')
idx = 0
c = Afaces
while idx < len(c):
  eol = ',' if idx+3 < len(c) else ''
  print (str(c[idx]) + ', ' + str(c[idx+1]) + ', ' + str(c[idx+2]) + eol)
  idx += 3
print ('};')
