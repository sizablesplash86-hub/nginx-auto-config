<?php
header('Content-Type: application/json');
// api.php
$input = json_decode(file_get_contents('php://input'), true);

$name = preg_replace('/[^a-zA-Z0-9_-]/', '', $input['name']);
$config = $input['config'];

// If PHP version auto-detection is needed on backend:
if (strpos($config, 'php8.2-fpm.sock') !== false) {
    // Find active php-fpm socket dynamically
    $socks = glob('/run/php/php*-fpm.sock');
    if (!empty($socks)) {
        $active_sock = $socks[0];
        $config = preg_replace('/unix:\/run\/php\/php.*?-fpm\.sock/', 'unix:' . $active_sock, $config);
    }
}

// Write file to sites-available and enable
$file_path = "/etc/nginx/sites-available/" . $name;
file_put_contents("/tmp/" . $name, $config);
exec("sudo mv /tmp/$name $file_path");
exec("sudo ln -sf $file_path /etc/nginx/sites-enabled/");
exec("sudo nginx -t", $out, $ret);

if ($ret === 0) {
    exec("sudo systemctl reload nginx");
    echo json_encode(['message' => "Configuration '$name' applied and NGINX reloaded successfully!"]);
} else {
    echo json_encode(['error' => "NGINX syntax check failed: " . implode("\n", $out)]);
}
