document.getElementById('revealButton').addEventListener('click', function() {
    // Hide the initial content
    document.getElementById('initial-content').style.display = 'none';
    
    // Show the surprise content
    document.getElementById('surprise-content').style.display = 'block';
    
    // Trigger confetti explosion
    confettiEffect();
});

function confettiEffect() {
    var duration = 5 * 1000;
    var end = Date.now() + duration;

    (function frame() {
        confetti({
            particleCount: 7,
            angle: 60,
            spread: 55,
            origin: { x: 0 }
        });
        confetti({
            particleCount: 7,
            angle: 120,
            spread: 55,
            origin: { x: 1 }
        });

        if (Date.now() < end) {
            requestAnimationFrame(frame);
        }
    }());
}
