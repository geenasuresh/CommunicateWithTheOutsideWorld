<?php require_once 'header.php'; ?>
<div role="main">
 <p><a href="/<?= urlencode($this->page); ?>">Cancel</a></p>
 <form action="/<?= urlencode($this->page); ?>" method="post">
 <textarea name="content"><?= htmlspecialchars($this->content); ?></textarea>
 <input type="submit" value="Save">
</div>
<?php require_once 'footer.php'; ?>
