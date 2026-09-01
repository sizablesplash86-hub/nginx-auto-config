<?php
header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true);

if (!isset($data['domain']) || !isset($data['config'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid input']);
    exit;
}

$domain = preg_replace('/[^a-zA-Z0-9\.\-]/', '', $data['domain']);
$config = $data['config'];

// Temporary write location accessible by www-data
$tmp_path = "/tmp/" . $domain . ".conf";
file_put_contents($tmp_path, $config);

// Call a helper script via sudo to deploy and reload
$command = "sudo /usr/local/bin/deploy-nginx-config " . escapeshellarg($tmp_path) . " " . escapeshellarg($domain);
exec($command, $output, $return_var);

if ($return_var === 0) {
    echo json_encode(['success' => true, 'message' => 'Configuration applied successfully!']);
} else {
    http_response_code(500);
    echo json_encode(['error' => 'Failed to apply configuration', 'details' => $output]);
}
