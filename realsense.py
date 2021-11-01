import pyrealsense2 as rs #asはCでいうところのdefine的役割
import numpy as np #C的なベクトルが使える
import time
import cv2
import serial  #mbedとの通信
from PIL import Image
from matplotlib import pyplot as plt
from datetime import datetime as dt

#ラッパーはC++を翻訳してpythonで使えるように
#WeWillMakeYouHappy ak-gakusei2
#7

com= input("what is com?:\n")

ser=serial.Serial("com%s"%(com),115200,timeout=0)#通信

prev_shot = dt.now()
prev_graph = dt.now()
INTERVAL_SHOT = 0 #sec
INTERVAL_GRAPH = 0.001


align = rs.align(rs.stream.color) #補正
aruco = cv2.aruco #マーカー識別のためのcv2のモジュール
p_dict = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)#事前定義されたマーカーの取得



#rs2opt=pyrealsense2.options()#設定を変える
#set_option(self: rs.options, option: rs .option, value: float)
#def set_option(self, option, value): 
  #laser_power=360
  #laser_power= option.laser_power
config = rs.config()#インスタンスの生成

config.enable_stream(rs.stream.color, 1280,720, rs.format.bgr8, 30)
config.enable_stream(rs.stream.depth, 1280,720, rs.format.z16, 30)
config.enable_stream(rs.stream.infrared,1, 1280,720, rs.format.y8, 30)#IRカメラ
pipeline = rs.pipeline()#インスタンスの生成
profile = pipeline.start(config)#この時にprofileが本体から渡される これによってprofileの文字が使えるようになる
#sensor.set_option(RS2_OPTION_LASER_POWER)
#frame_laser_power =360.frame_laser_power
#depth_scale = profile.get_device().first_depth_sensor().get_depth_scale()#未使用

#以下レーザー出力の設定
#pipeline_profile = pipeline.start(config)
set_laser = 180
device = profile.get_device()
depth_sensor = device.query_sensors()[0]#どのセンサーか
depth_sensor.set_option(rs.option.laser_power, set_laser)#ここで書き込んでいる
laser_pwr = depth_sensor.get_option(rs.option.laser_power)#読み込み
laser_range = depth_sensor.get_option_range(rs.option.laser_power)#読み込み
#just simply add 10 to test set function
#if laser_pwr + 10 > laser_range.max:
# set_laser = laser_range.max
#else:
#  set_laser = laser_pwr + 10
print("laser power = ", laser_pwr)
print("laser range =",laser_range)

#グラフ生成
fig, ax = plt.subplots()
ax2=ax.twinx()
#x軸の幅を1000にする
xlim = [0, 10]
time=[]
xservoAngleArray,xmotorAngleArray,xmotorPowerArray,yservoAngleArray,ymotorAngleArray,ymotorPowerArray = [], [], [], [], [], []
c1,c2,c3,c4,c5,c6 = 'g','b','y','m' ,'r','k'    # 各プロットの色
l1,l2,l3,l4,l5,l6 = "xservoAngle","xmotorAngle","xmotorPower","yservoAngle","ymotorAngle" ,"ymotorPower"  # 各ラベル



try:#例外にかかわる処理　tryの中に例外があるとtryを終える。例外が起こった時にexceptがあるとそっちに飛ぶ。expectで処理しきれないと停止する。
  while True:#無限ループ

    cur = dt.now()

    if (cur - prev_shot).total_seconds() >= INTERVAL_SHOT:
      prev_disp = cur

      frames = pipeline.wait_for_frames()#フレーム待ち（画像たちが送られるのを待つ？）
      aligned_frames = align.process(frames)#修正
      color_frame = aligned_frames.get_color_frame()#RGBカメラ
      depth_frame = aligned_frames.get_depth_frame()#深度
      #print(depth_frame)
      ir_frame1 = frames.get_infrared_frame(1)#ir映像
        
      if not depth_frame or not color_frame:
        continue
    
      #フィルター（未完成）
      #dec_filter = rs.decimation_filter (2)#圧縮する　()内は２から８
      #spat_filter = rs.spatial_filter()
      #temp_filter = rs.temporal_filter()
      #pers_filter = rs.persistence_filter()
      #hole_filter = rs.hole_filling_filter()
      #spatial.set_option(rs.option.holes_fill, 3)

      #spat_filter(rs.option.holes_fill, 3) 
      #depth_frame = dec_filter.process(depth_frame)
      #depth_frame = spat_filter.process(depth_frame)
      #depth_frame = temp_filter.process(depth_frame)
      #depth_frame = hole_filter.process(depth_frame)
      #depth_frame = depth_to_disparity.process(depth_frame)
      #depth_frame = disparity_to_depth.process(depth_frame)
      #depth_frame = hole_filling.process(depth_frame)
      #depth_frame = spatial.process(depth_frame)

      color_image = np.asanyarray(color_frame.get_data()) #colorをnumpy配列に変換
      ir_image1 = np.asanyarray(ir_frame1.get_data())#irをnumpyに変換
      depth_frame_processing = rs.colorizer().colorize(depth_frame)#サーモグラフィーみたいにする
      depth_image = np.asanyarray(depth_frame_processing.get_data())#depthをnumpyに変換

      color_igame = color_image[:,:,::-1]#RGBとBRGの変換 スライサーの機能で動いてる
      depth_color_frame = rs.colorizer().colorize(depth_frame)# Depth画像に変換1
      depth_color_image = np.asanyarray(depth_color_frame.get_data())# Depth画像に変換2
      video_stream_prof=rs.video_stream_profile(profile.get_stream(rs.stream.color))# videostreamprofile　継承のあたりを読もう！
      color_intr = video_stream_prof.get_intrinsics()#パラメーターの取得,インスタンスを作っている。
      #型の表示print(type(rs.video_stream_profile(profile.get_stream(rs.stream.color))))
    
    
      #cv2でのマーカー処理
      #color_image = cv2.cvtColor(color_image, cv2.COLOR_RGB2GRAY)#モノクロ化
      corners, ids, rejectedImgPoints = aruco.detectMarkers(color_image, p_dict) # マーカーの検出 rejectedimgpoints=デバック用
      ids  = np.ravel(ids)#平滑化
      #print(len(ids))#要素数の取得
      if ids[0]==0:#idsの0番目のみ見る 
        color_image = aruco.drawDetectedMarkers(color_image.copy(), corners, ids)# 検出結果をオーバーレイ ids=マーカー番号
        corners  = np.ravel(corners)#平滑化
        #uplefdepth=depth_frame.get_distance(corners[4], corners[5])#左上の座標について　
        #uplef= rs.rs2_deproject_pixel_to_point(color_intr , [corners[4], corners[5]], uplefdepth)
        #print(uplef)
        center = [int((corners[5]+corners[1])/2),int((corners[4]+corners[0])/2)]#中心の計算
        cv2.drawMarker(color_image, (center[1], center[0]), (0, 255, 0), markerType=cv2.MARKER_CROSS, markerSize=30)#中心にマーカー描写
        center.append(depth_frame.get_distance(center[1], center[0]))  #深さの取得 center配列に追加
        #print(color_intr)
        xyz = rs.rs2_deproject_pixel_to_point(color_intr , [center[1], center[0]],  center[2])#座標情報(x,y)と深度情報とカメラモジュールの内部パラメータからカメラを中心とした3次元座標
        #print(xyz[2])
        #xyz[2]= (((xyz[2]**2)-(xyz[1]**2))-(xyz[0])**2)**0.5#三平方の定理から奥行求める
        #print(xyz)
        #cap = cv2.VideoCapture(0)
        #print(cap.get(cv2.CAP_PROP_FPS))
        #fps=video_stream_prof.fps()
        #print("FPS=",fps)
        for i in range(3):
          center[0]+=15
          cv2.putText(color_image,str(xyz[i]),  (center[1], center[0]), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,255), thickness=1)#xyzの描写 strでテキストに変換
      
        #corners[0]-=200
        #corners[1]-=200
        #corners[4]+=200
        #corners[5]+=200
        #expansion_color_image = color_image[int(corners[1]) : int(corners[5]) , int(corners[0]) : int(corners[4])]#[top : bottom , left : right] 風船識別のトリミング  intでないとダメ
        #cv2.imshow('expansion_bw_image',expansion_color_image)#表示
        #cv2.waitKey(1)#1msだけ

        #以下送信
        #これで送信
        willsend=('{:+.10f}'.format(xyz[0])+"\0"+'{:+.10f}'.format(xyz[1])+"\0"+'{:+.10f}'.format(xyz[2])+"\0"+"\n")#+-をつけて小数点以下10桁（こんなにいる？）
        #print(willsend)
        encodedVal=(willsend).encode()
      
        ser.write(encodedVal)
      #cv2.namedWindow('color_image', cv2.WINDOW_NORMAL)#フルスクリーン
      #cv2.setWindowProperty('color_image', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)#フルスクリ-ン
      cv2.imshow('color_image',color_image)#表示
      #cv2.imshow('depth_image',depth_image)#表示
      #cv2.imshow('ir_frame1',ir_image1)#表示
      cv2.waitKey(1)  
  
    if (cur - prev_graph).total_seconds() >= INTERVAL_GRAPH:
      prev_disp = cur

		  #以下受信
      line=ser.readline()#'\n'まで読み、'\n'込みのbyte列を返す

      if line!=b'':#バイトがたの配列1byte
      
			  #byte列をdecodeして、'\n'を削除する場合 +00.00+00.00+0.0000+00.00+00.00+0.0000     
        receive=line.decode().replace("\n","")
        print(receive)
        if len(receive)==-1:   
          xservoAngle=float(receive[0:5])
          print(xservoAngle)
          xmotorAngle=float(receive[6:11])
          xmotorPower=float(receive[12:18])
          yservoAngle=float(receive[19:24])
          ymotorAngle=float(receive[25:30])
          ymotorPower=float(receive[31:37])
          #byte列のままの場合
   
          xservoAngleArray.append(xservoAngle)
          xmotorAngleArray.append(xmotorAngle)
          xmotorPowerArray.append(xmotorPower)
          yservoAngleArray.append(yservoAngle)
          ymotorAngleArray.append(ymotorAngle)
          ymotorPowerArray.append(ymotorPower)

          time.append(len(xservoAngleArray))
          #timeに100個以上格納されたら
          if len(time) > 10:
                  xlim[0] += 1
                  xlim[1] += 1

          ax.plot(time, xservoAngleArray, color=c1, label=l1)
          ax.plot(time, xmotorAngleArray, color=c2, label=l2)
          ax2.plot(time, xmotorPowerArray, color=c3, label=l3)
          ax.plot(time, yservoAngleArray, color=c4, label=l4)
          ax.plot(time, ymotorAngleArray, color=c5, label=l5)
          ax2.plot(time, ymotorPowerArray, color=c6, label=l6)



          #x軸、y軸方向の表示範囲を再設定(左に動かすため）
          ax.set_ylim(-90, 90)
          ax2.set_ylim(-1,1)
          ax.set_xlim(xlim[0], xlim[1])
          ax2.set_xlim(xlim[0], xlim[1])
          
          #描写
          #plt.pause(0.1)
          #ax.cla()
          #ax2.cla()

    
    


    


except KeyboardInterrupt:
  pipeline.stop()
  cv2.destroyAllWindows()
  ser.close()





    
    
"""
      corners0 = corners[list(ids.flatten()).index(0)][0][0]
      corners2 = corners[list(ids.flatten()).index(0)][0][2]
      corners0[0]-=50
      corners2[0]-=50
      corners0[1]+=50
      corners2[1]+=50
      expansion_bw_image = bw_image[corners0[0]: corners0[0], corners0[1]: corners2[1]]#風船識別のトリミング
      cv2.imshow('expansion_bw_image',expansion_bw_image)#表示
      cv2.waitKey(1)#1msだけ
      print(corners)

"""

