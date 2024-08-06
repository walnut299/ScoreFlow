        var studentModal = document.getElementById("studentModal");
        var gradeModal = document.getElementById("gradeModal");
        var studentBtn = document.getElementById("openStudentModal");
        var gradeBtn = document.getElementById("openGradeModal");
        var closeBtns = document.getElementsByClassName("close");

        studentBtn.onclick = function() {
            studentModal.style.display = "block";
        }

        gradeBtn.onclick = function() {
            gradeModal.style.display = "block";
        }

        for (var i = 0; i < closeBtns.length; i++) {
            closeBtns[i].onclick = function() {
                studentModal.style.display = "none";
                gradeModal.style.display = "none";
            }
        }

        window.onclick = function(event) {
            if (event.target == studentModal) {
                studentModal.style.display = "none";
            }
            if (event.target == gradeModal) {
                gradeModal.style.display = "none";
            }
        }