package com.domain17.glflight;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.res.*;
import android.os.Environment;
import android.media.*;

public class GameResources {
	
	public static native void glFlightGameResourceInit(String s);
	public static native void glFlightGameResourceRead(String s);
	
	// this needs to be bumped every time you add/modify a resource
	// so that the java code unpacks resource so that JNI code can reach it
	public static int version = /* TODO: use android-version-string */ 10117;
	
	public static String resPrefix = /*"gameResources/"*/ "";
	public static String resVersionFilename = "resVersion";
	
	public static Integer[] resObj = {
		//R.drawable.abc_ab_bottom_solid_light_holo,

        R.raw.texture0,
        R.raw.texture1,
        R.raw.texture2,
        R.raw.texture3,
        R.raw.texture4,
        R.raw.texture5,
        R.raw.texture6,
        R.raw.texture7,
        R.raw.texture8,
        R.raw.texture9,
        R.raw.texture10,
        R.raw.texture11,
        R.raw.texture12,
        R.raw.texture13,
        R.raw.texture14,
        R.raw.texture15,
        R.raw.texture16,
        R.raw.texture17,
        R.raw.texture18,
        R.raw.texture19,
        R.raw.texture20,
        R.raw.texture21,
        R.raw.texture22,
        R.raw.texture23,
        R.raw.texture24,
        R.raw.texture25,
        R.raw.texture26,
        R.raw.texture27,
        R.raw.texture28,
        R.raw.texture29,
        R.raw.texture30,
        R.raw.texture31,
        R.raw.texture32,
        R.raw.texture33,
        R.raw.texture34,
        R.raw.texture35,
        R.raw.texture36,
        R.raw.texture37,
        R.raw.texture38,
        R.raw.texture39,
        R.raw.texture40,
        R.raw.texture41,
        R.raw.texture42,
        R.raw.texture43,
        R.raw.texture44,
        R.raw.texture45,
        R.raw.texture46,
        R.raw.texture47,
        R.raw.texture48,
        R.raw.texture49,
        R.raw.texture50,
        R.raw.texture51,
        R.raw.texture52,
        R.raw.texture53,
        R.raw.texture54,
        R.raw.texture55,
        R.raw.texture56,
        R.raw.texture57,
        R.raw.texture58,
        R.raw.texture59,
        R.raw.texture60,
        R.raw.texture61,
        R.raw.texture62,
        R.raw.texture63,
        R.raw.texture64,
        R.raw.texture65,
        R.raw.texture66,
        R.raw.texture67,
        R.raw.texture68,
        R.raw.texture69,
        R.raw.texture70,
        R.raw.texture71,
        R.raw.texture72,
        R.raw.texture73,
        R.raw.texture74,
        R.raw.texture75,
        R.raw.texture76,
        R.raw.texture77,
        R.raw.texture78,
        R.raw.texture79,
        R.raw.texture80,
        R.raw.texture81,
        R.raw.texture82,
        R.raw.texture83,
        R.raw.texture84,
        R.raw.texture85,
        R.raw.texture86,
        R.raw.texture87,
        R.raw.texture88,
        R.raw.texture89,
        R.raw.texture90,
        R.raw.texture91,
        R.raw.texture92,
        R.raw.texture93,
        R.raw.texture94,
        R.raw.texture95,
        R.raw.texture96,
        R.raw.texture97,
        R.raw.texture98,
        R.raw.texture99,
        R.raw.texture100,
        R.raw.texture101,
        R.raw.texture102,
        R.raw.texture103,
	};
	
	public static String[] resFilename = {
		//"abc_ab_bottom_solid_light_holo.png",
		
        "texture0.bmp",
        "texture1.bmp",
        "texture2.bmp",
        "texture3.bmp",
        "texture4.bmp",
        "texture5.bmp",
        "texture6.bmp",
        "texture7.bmp",
        "texture8.bmp",
        "texture9.bmp",
        "texture10.bmp",
        "texture11.bmp",
        "texture12.bmp",
        "texture13.bmp",
        "texture14.bmp",
        "texture15.bmp",
        "texture16.bmp",
        "texture17.bmp",
        "texture18.bmp",
        "texture19.bmp",
        "texture20.bmp",
        "texture21.bmp",
        "texture22.bmp",
        "texture23.bmp",
        "texture24.bmp",
        "texture25.bmp",
        "texture26.bmp",
        "texture27.bmp",
        "texture28.bmp",
        "texture29.bmp",
        "texture30.bmp",
        "texture31.bmp",
        "texture32.bmp",
        "texture33.bmp",
        "texture34.bmp",
        "texture35.bmp",
        "texture36.bmp",
        "texture37.bmp",
        "texture38.bmp",
        "texture39.bmp",
        "texture40.bmp",
        "texture41.bmp",
        "texture42.bmp",
        "texture43.bmp",
        "texture44.bmp",
        "texture45.bmp",
        "texture46.bmp",
        "texture47.bmp",
        "texture48.bmp",
        "texture49.bmp",
        "texture50.bmp",
        "texture51.bmp",
        "texture52.bmp",
        "texture53.bmp",
        "texture54.bmp",
        "texture55.bmp",
        "texture56.bmp",
        "texture57.bmp",
        "texture58.bmp",
        "texture59.bmp",
        "texture60.bmp",
        "texture61.bmp",
        "texture62.bmp",
        "texture63.bmp",
        "texture64.bmp",
        "texture65.bmp",
        "texture66.bmp",
        "texture67.bmp",
        "texture68.bmp",
        "texture69.bmp",
        "texture70.bmp",
        "texture71.bmp",
        "texture72.bmp",
        "texture73.bmp",
        "texture74.bmp",
        "texture75.bmp",
        "texture76.bmp",
        "texture77.bmp",
        "texture78.bmp",
        "texture79.bmp",
        "texture80.bmp",
        "texture81.bmp",
        "texture82.bmp",
        "texture83.bmp",
        "texture84.bmp",
        "texture85.bmp",
        "texture86.bmp",
        "texture87.bmp",
        "texture88.bmp",
        "texture89.bmp",
        "texture90.bmp",
        "texture91.bmp",
        "texture92.bmp",
        "texture93.bmp",
        "texture94.bmp",
        "texture95.bmp",
        "texture96.bmp",
        "texture97.bmp",
        "texture98.bmp",
        "texture99.bmp",
        "texture100.bmp",
        "texture101.bmp",
        "texture102.bmp",
        "texture103.bmp",
	};
	
	public static String[] sndNameList = {
		"boom",
		"bump",
		"collect",
		"dead",
		"dropoff",
		"engine",
		"engineSlow",
		"engineFast",
		"flyby",
		"highscore",
		"lockedon",
		"missle",
		"shoot",
		"speedboost",
		"towing",
		"victory",
		"warning",
		"teleport"
		
	};
	
	public static int[] sndIdList = {
		R.raw.boom,
		R.raw.bump,
		R.raw.collect,
		R.raw.dead,
		R.raw.dropoff,
		R.raw.engine,
		R.raw.engineslow,
		R.raw.enginefast,
		R.raw.flyby,
		R.raw.highscore,
		R.raw.lockedon,
		R.raw.missle,
		R.raw.shoot,
		R.raw.speedboost,
		R.raw.towing,
		R.raw.victory,
		R.raw.warning,
        R.raw.teleport
	};
	
	static GameResources inst = null;
	
	Map<String, Integer> soundMap = new HashMap<String, Integer>();
	
	SoundPool soundPool = new SoundPool(16, AudioManager.STREAM_MUSIC, 0);

	public static int init(Context c) {
		
		if(inst == null) {
			inst = new GameResources();
		}
		
		// unpack resources where NDK can reach them
		// TODO: check version doesn't match, only unpack when changed
		
		if(resObj.length != resFilename.length)
		{
            System.out.println("resObj/resFilename out-of-sync");
			return 0;
		}
		
		System.out.println("init resources");


		String envPrefix = c.getExternalCacheDir() + "/";
		/*Environment.getExternalStorageDirectory().getAbsolutePath() + "/";*/
		
		glFlightGameResourceInit(envPrefix);
		
        Resources res = c.getResources();
	
		int i;
		for(i = 0; i < resObj.length; i++)
		{
	
		 try {
		        InputStream in_s = res.openRawResource(resObj[i]);

		        byte[] b = new byte[in_s.available()];
		        in_s.read(b);
		        in_s.close();
		        
		        String fileName = envPrefix + resFilename[i];
		        
		        System.out.println("GameResources: loading " + resFilename[i] + " (len:" + b.length + ") to " + fileName);
		        
		        FileOutputStream out_s = new FileOutputStream(fileName);
		        out_s.write(b);
		        out_s.close();
		    } catch (Exception e) {
		        e.printStackTrace();
		    }
		}
		
		/* sounds */
		for(i = 0; i < sndNameList.length; i++)
		{
			inst.soundMap.put(sndNameList[i], Integer.valueOf(inst.soundPool.load(c, sndIdList[i], 1)));
		}
		
		return 1;
	}

	public static void playSound(String name) {
		if(inst.soundMap.containsKey(name)) {
			inst.soundPool.play(inst.soundMap.get(name), 1.0f, 1.0f, 1, 1, 1.0f);
		}
	}
}
