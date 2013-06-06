<?php
require_once '../vendor/autoload.php';

// Common resources used by routes: configuration, database, template renderer,
// and Slim instance

$cfg = require '../config/config.php';

$pdo = new PDO($cfg['db.driver'] . ':' . $cfg['db.filename']);
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
$db = new NotORM($pdo);

$template = new Template(dirname(__FILE__) . '/../templates/');

$app = new Slim\Slim();

// Routes

// redirect to the start page if a specific page is not requested by the user
$app->get(
    '/',
    function () use ($app) {
        $app->redirect('/home');
    }
);

// attempt to retrieve and display a specific requested page
$app->get(
    '/:page',
    function ($page) use ($cfg, $db, $template) {
        $row = $db->page_content('page', $page)->select('content')->fetch();

        // process content using proc_open() to pass it to the markup engine
        $content = '';
        if ($row['content']) {
            // descriptor array
            $desc = array(
                0 => array('pipe', 'r'), // 0 is STDIN for process
                1 => array('pipe', 'w'), // 1 is STDOUT for process
                2 => array('file', '/tmp/error-output.txt', 'a') // 2 is STDERR
            );

            // command to invoke markup engine 
            $cmd = $cfg['path.nme'] . ' --strictcreole --autourllink --body --xref';

            // spawn the process
            $p = proc_open($cmd, $desc, $pipes);

            // send the wiki content as input to the markup engine and then
            // close the input pipe so the engine knows not to expect more
            // input and can start processing
            fwrite($pipes[0], $row['content']);
            fclose($pipes[0]);

            // read the output from the engine
            $content = stream_get_contents($pipes[1]);

            // close the output pipes and process
            fclose($pipes[1]);
            fclose($pipes[2]);
            proc_close($p);
        }

        $template->page = $page;
        $template->content = $content;

        // if there's content then render the page, otherwise empty content
        // means a page "doesn't exist" so show the create form instead
        $template->fetch(($content) ? 'view.php' : 'create.php');
    }
);

// create or update content in the database
$app->post(
    '/:page',
    function ($page) use ($app, $db, $template) {
        $db->page_content->insert_update(
            array('page' => $page),
            array('content' => $_POST['content']), // insert
            array('content' => $_POST['content'])  // update if exists
        );

        $app->redirect('/' . $page);
    }
);

// display the content in a form for the user to edit
$app->get(
    '/:page/edit',
    function ($page) use ($db, $template) {
        $row = $db->page_content('page', $page)->select('content')->fetch();

        $template->page = $page;
        $template->content = $row['content'];

        $template->fetch('edit.php');
    }
);

$app->run();
