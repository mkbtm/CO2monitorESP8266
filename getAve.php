<?php

date_default_timezone_set(‘Asia/Tokyo’);//タイムゾーンの設定

//文字コードの設定
mb_internal_encoding(“UTF-8”);

///http://mkbtm.net/co2/getLastOnueHour.php?year=2020&month=7&day=4&hour=11&min=10&id=1

$year = $_GET[“year”];
$month = $_GET[“month”];
$day = $_GET[“day”];
$hour = $_GET[“hour”];
$min = $_GET[“min”];
$id = $_GET[“id”];

//現在の日付と時刻を取得
//$nowDate = date(“Y/m/d G:i”);
//受け取った時刻からUNIXタイムへ変換する
$dayTimeStr = $year . “/” . $month . “/” . $day . “ “ . $hour . “:” . $min . “:00”;
$nowDateTimeUNIX = strtotime($dayTimeStr);


$fileName = “./data/log_” . $id . “.csv”;
$array = file($fileName);//ファイルの中身を配列にいれる。

$i = 0;
$returnString = “”;
$totalTemp = 0.0;
$totalCO2 = 0.0;
$totalHum = 0.0;
$count = 0;
$aveTemp=0.0;
$aveCO2=0.0;
$aveHum=0.0;


foreach ($array as $line) {
    $lineArray = explode(‘,’, $line);
    $logDate = $lineArray[0];
    $logCO2 = $lineArray[1];
    $logTemp = $lineArray[2];
    $logHum = $lineArray[3];

    $logHum = str_replace(“\n”, ‘’, $logHum);//最後の改行を削除している

    $logDateTimeUNIX = strtotime($logDate);
   

    $pastOneHour = $nowDateTimeUNIX-3600;//指定された時間よりも1時間前のUNIX time

    if (($pastOneHour < $logDateTimeUNIX) ){
        //echo $logDate . “<br>” ; 
        $count++;
        $totalTemp += $logTemp;
        $totalCO2 += $logCO2;
        $totalHum += $logHum;
        
    }
}
/*echo $totalCO2 . “<br>”;
echo $totalTemp . “<br>”;
echo $totalHum . “<br>”;*/


if ($count !=0){
    $aveTemp += $totalTemp/$count;
    $aveCO2 += $totalCO2/$count;
    $aveHum += $totalHum/$count;
}
//レスポンス

header(“Content-type: text/html”);
echo $id,”,” . $aveCO2 . “,” . $aveTemp . “,” . $aveHum;i
