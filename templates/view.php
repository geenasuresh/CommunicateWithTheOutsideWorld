<?php require_once 'header.php'; ?>
<div role="main">
<p><a href="/<?= urlencode($this->page); ?>/edit">Edit</a></p>
<?= $this->content; ?>
</div>
<?php require_once 'footer.php'; ?>
