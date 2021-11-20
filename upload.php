<?php

date_default_timezone_set(‘Asia/Tokyo’);//タイムゾーンの設定

//文字コードの設定
mb_internal_encoding(“UTF-8”);

$dir = “./data/*”;
$returnString = “”;
foreach(glob($dir) as $file){
    if(is_file($file)){
        $returnString = $returnString  . $file . “,”;
        //echo htmlspecialchars($file);
    }
}

echo $returnString;
