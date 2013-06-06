<?php require_once 'header.php'; ?>
<div role="main">
 <p>Uh oh... it looks like <strong><?= htmlspecialchars($this->page); ?></strong> doesn't exist. Let's create it!
 <form action="/<?= urlencode($this->page); ?>" method="post">
 <textarea name="content"></textarea>
 <input type="submit" value="Create">
</div>
<?php require_once 'footer.php'; ?>
