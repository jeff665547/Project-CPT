<?php 
function get_client_os ( &$user_agent ) { 
    $os_platform    =   "Unknown OS Platform";
    $os_array       =   array(
        '/windows nt 10/i'      =>  'Windows 10',
        '/windows nt 6.3/i'     =>  'Windows 8.1',
        '/windows nt 6.2/i'     =>  'Windows 8',
        '/windows nt 6.1/i'     =>  'Windows 7',
        '/windows nt 6.0/i'     =>  'Windows Vista',
        '/windows nt 5.2/i'     =>  'Windows Server 2003/XP x64',
        '/windows nt 5.1/i'     =>  'Windows XP',
        '/windows xp/i'         =>  'Windows XP',
        '/windows nt 5.0/i'     =>  'Windows 2000',
        '/windows me/i'         =>  'Windows ME',
        '/win98/i'              =>  'Windows 98',
        '/win95/i'              =>  'Windows 95',
        '/win16/i'              =>  'Windows 3.11',
        '/macintosh|mac os x/i' =>  'Mac OS X',
        '/mac_powerpc/i'        =>  'Mac OS 9',
        '/linux/i'              =>  'Linux',
        '/ubuntu/i'             =>  'Ubuntu',
        '/iphone/i'             =>  'iPhone',
        '/ipod/i'               =>  'iPod',
        '/ipad/i'               =>  'iPad',
        '/android/i'            =>  'Android',
        '/blackberry/i'         =>  'BlackBerry',
        '/webos/i'              =>  'Mobile'
    );
    foreach ($os_array as $regex => $value) { 
        if (preg_match($regex, $user_agent)) {
            $os_platform    =   $value;
        }
    }   
    return $os_platform;
}
function gen_file_posfix( $os )
{
    if ( preg_match( '/Win/i', $os ) )
    {
        return "win.7z";
    }
    else if ( preg_match( '/Linux/i', $os ) )
    {
        return "linux.tar.gz";
    }
    else
    {
        return "win.7z";
    }
}

# get latest version file 
$verfile = fopen( "LATEST_VERSION", 'r' );
$latest_version = trim(fgets($verfile));

# get client require version
if ( isset($_GET["version"]) )
{
    $version = $_GET["version"];
}
else
{
    $version = $latest_version;
}

# get platform postfix
if ( isset($_GET["os"]) )
{
    $os = $_GET["os"];
}
else
{
    $os = get_client_os($_SERVER['HTTP_USER_AGENT']);
}

# generate file posfix
$file_posfix = gen_file_posfix( $os );

# generate url
# echo $latest_version;
# echo $version;
# echo strcmp ( $version, $latest_version );
if ( is_null( $version ) || $version == "latest" || strcmp ( $version, $latest_version ) == 0 )
{
    $url = "CPT_binary_" . $file_posfix ;
}
else
{
    $url = $version . "/CPT_binary_" . $file_posfix;
}

?>
<meta http-equiv="refresh" content="0; url=<?php echo $url; ?>" />

