#ifndef __maps_macro_h__
#define __maps_macro_h__

/*
 (Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id)
*/
#define WORLD_ADD_OBJECT(model , x, y, z, yaw, pitch, roll, scale, tx_id) \
"add_object " #model " "#x" "#y" "#z" "#yaw" "#pitch" "#roll" "#scale" "#tx_id"\n"

#define M_(x) #x
#define M(x) (M_(x)-OBJ_ZERO)
#define WORLD_OBJECT_TYPE(e) M(e)

#endif
