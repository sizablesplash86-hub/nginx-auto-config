<?php
header('Content-Type: application/json');

// Read raw JSON body from fetch request
$json_data = file_get_contents('php://input');
$data = json_decode($json_data, true);

if (!$data || empty($data['name'])) {
    echo json_encode(['error' => 'Invalid configuration data provided.']);
    exit;
}

// Write the parameters to a temporary JSON file
$json_file = '/tmp/nginx_config_input.json';
file_put_contents($json_file, json_encode($data, JSON_PRETTY_PRINT));

// Execute the C binary passing the JSON file path
$cmd = "sudo /usr/local/bin/nginx-auto --json " . escapeshellarg($json_file) . " 2>&1";
exec($cmd, $output, $return_var);

if ($return_var === 0) {
    echo json_encode([
        'status' => 'success',
        'message' => 'Configuration generated and applied successfully!'
    ]);
} else {
    echo json_encode([
        'status' => 'error',
        'error' => implode("\n", $output)
    ]);
}
?>
