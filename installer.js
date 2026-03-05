/**
 * installer.js — Step-by-step Flipper Zero installer & tutorial logic
 */

(function () {
  'use strict';

  // ── Sticky nav ───────────────────────────────────────────────────
  const nav = document.querySelector('nav');
  window.addEventListener('scroll', () => {
    nav.classList.toggle('scrolled', window.scrollY > 40);
  });

  // ── Installer stepper ────────────────────────────────────────────
  const steps = Array.from(document.querySelectorAll('.step'));
  let currentStep = 0;

  function setStep(index) {
    steps.forEach((step, i) => {
      step.classList.remove('active', 'done');
      const dot = step.querySelector('.step-dot');
      dot.textContent = i + 1;

      if (i < index) {
        step.classList.add('done');
        dot.textContent = '';            // checkmark via CSS ::after
      } else if (i === index) {
        step.classList.add('active');
      }
    });
    currentStep = index;
    updateProgress();
  }

  function updateProgress() {
    const pct = steps.length > 1
      ? (currentStep / (steps.length - 1)) * 100
      : 0;
    const bar = document.querySelector('.progress-bar');
    if (bar) bar.style.width = pct + '%';
  }

  // Clicking a step header activates it
  steps.forEach((step, i) => {
    const title = step.querySelector('.step-title');
    if (title) {
      title.addEventListener('click', () => setStep(i));
    }
    const dot = step.querySelector('.step-dot');
    if (dot) {
      dot.addEventListener('click', () => setStep(i));
    }
  });

  // "Next" buttons inside steps
  document.querySelectorAll('[data-next-step]').forEach((btn) => {
    btn.addEventListener('click', () => {
      const next = parseInt(btn.dataset.nextStep, 10);
      if (next < steps.length) setStep(next);
    });
  });

  // Initialise with step 0 active
  setStep(0);

  // ── FAQ accordion ────────────────────────────────────────────────
  document.querySelectorAll('.faq-item').forEach((item) => {
    const q = item.querySelector('.faq-q');
    if (!q) return;
    q.addEventListener('click', () => {
      const wasOpen = item.classList.contains('open');
      document.querySelectorAll('.faq-item').forEach((el) => el.classList.remove('open'));
      if (!wasOpen) item.classList.add('open');
    });
  });

  // ── Smooth-scroll nav links ──────────────────────────────────────
  document.querySelectorAll('a[href^="#"]').forEach((link) => {
    link.addEventListener('click', (e) => {
      const target = document.querySelector(link.getAttribute('href'));
      if (target) {
        e.preventDefault();
        target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      }
    });
  });

  // ── "Copy to clipboard" on code blocks ──────────────────────────
  document.querySelectorAll('.step-code').forEach((block) => {
    block.style.cursor = 'pointer';
    block.title = 'Click to copy';
    block.addEventListener('click', () => {
      navigator.clipboard.writeText(block.textContent.trim()).then(() => {
        const orig = block.style.borderLeftColor;
        block.style.borderLeftColor = '#39FF14';
        setTimeout(() => { block.style.borderLeftColor = orig; }, 1200);
      }).catch(() => {
        // Clipboard API unavailable or denied — highlight the text so the
        // user can copy manually with Ctrl+C / Cmd+C.
        const sel = window.getSelection();
        const range = document.createRange();
        range.selectNodeContents(block);
        sel.removeAllRanges();
        sel.addRange(range);
        block.title = 'Text selected — press Ctrl+C to copy';
      });
    });
  });

  // ── Scroll-reveal (lightweight Intersection Observer) ────────────
  const revealEls = document.querySelectorAll('.card, .tutorial-card, .faq-item');
  const observer = new IntersectionObserver(
    (entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          entry.target.style.opacity = '1';
          entry.target.style.transform = 'translateY(0)';
          observer.unobserve(entry.target);
        }
      });
    },
    { threshold: 0.12 }
  );

  revealEls.forEach((el) => {
    el.style.opacity = '0';
    el.style.transform = 'translateY(24px)';
    el.style.transition = 'opacity 0.55s ease, transform 0.55s ease';
    observer.observe(el);
  });
})();
