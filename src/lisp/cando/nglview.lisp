
(in-package :cando)

(defun safe-pick-history (widget)
  (funcall (find-symbol "PICK-HISTORY" :NGLV) widget))

(defun safe-add-shape (widget parts &key name)
  (funcall (find-symbol "ADD-SHAPE" :NGLV) widget parts :name name))


(defconstant +width+ 1.0)
(defconstant +half-width+ (/ +width+ 2.0))
(defun build-hydrogens (matter)
  (chem:map-atoms nil
                  (lambda (a)
                    (when (eq (chem:get-element a) :H)
                      (let* ((neighbors (chem:bonds-as-list a))
                             (_         (unless (eql (length neighbors) 1)
                                          (error "There are too many neighbors: ~a" neighbors)))
                             (neighbor (chem:get-other-atom (first neighbors) a))
                             (neighbor-pos (chem:get-position neighbor))
                             (pos (geom:vec
                                   (+ (- (random +width+) +half-width+) (geom:vx neighbor-pos))
                                   (+ (- (random +width+) +half-width+) (geom:vy neighbor-pos))
                                   (+ (- (random +width+) +half-width+) (geom:vz neighbor-pos)))))
                        (chem:set-position a pos))))
                  matter))

(defun npicked (widget n)
    (with-output-to-string (sout)
  (mapcar (lambda (a) 
                  (format sout ".~a "(cdr (assoc "atomname" (cdr (car a)) :test #'equal)))) 
          (subseq (safe-pick-history widget) 0 n))))


(defun pick-property (pick property)
  (cdr (assoc property (cdr (car pick)) :test #'equal)))

(defun atom-with-name (matter name)
  (let (atom)
    (chem:map-atoms 'nil (lambda (a)
                           (when (eq (chem:get-name a) name)
                             (setf atom a)))
                    matter)
    atom))

(defun atom-map (widget n matter-from matter-to)
  (unless (>= (length (safe-pick-history widget)) (* 2 n))
    (error "There aren't enough atoms selected to create a map for ~a pairs." n))
  (let ((map (loop for (pick-to pick-from) on (safe-pick-history widget) by #'cddr
                   for atom-from = (atom-with-name matter-from (intern (pick-property pick-from "atomname") :keyword))
                   for atom-to = (atom-with-name matter-to (intern (pick-property pick-to "atomname") :keyword))
                   collect (cons atom-from atom-to))))
    (subseq map 0 n)))

(defun named-atom-map (widget n matter-from matter-to)
  (let ((atom-map (atom-map widget n matter-from matter-to)))
    (loop for (from . to) in atom-map
          collect (cons (chem:get-name from) (chem:get-name to)))))


(defun anchor-named-atom-map (name-atom-map matter-from matter-to)
  (loop for (from-name . to-name) in name-atom-map
        for from-atom = (chem:first-atom-with-name matter-from from-name)
        for to-atom = (chem:first-atom-with-name matter-to to-name)
        for to-position = (chem:get-position to-atom)
        do (cando:anchor-atom from-atom to-position)))


(defun coord-to-vector (c)
  (vector (float (geom:vx c) 1.0s0)
          (float (geom:vy c) 1.0s0)
          (float (geom:vz c) 1.0s0)))

(defun arrow (from-vec to-vec &optional (radius 0.2))
  (let ((from-coord (coord-to-vector from-vec))
        (to-coord (coord-to-vector to-vec)))
    (vector "arrow" from-coord to-coord #(1 0 1) radius)))
(defun cartoon-atom-map-arrows (atom-map from-matter to-matter)
  (let (arrows)
    (loop for pair in atom-map
          for from-name = (car pair)
          for to-name = (cdr pair)
          for from-atom = (atom-with-name from-matter from-name)
          for to-atom = (atom-with-name to-matter to-name)
          for from-pos = (chem:get-position from-atom)
          for from-coord = (coord-to-vector from-pos)
          for to-pos = (chem:get-position to-atom)
          for to-coord = (coord-to-vector to-pos)
          for arrow = (vector "arrow" from-coord to-coord #(1 0 1) 0.2)
;;;          do (format t "Names:  ~a -> ~a   atoms:  ~a -> ~a~%" from-name to-name from-atom to-atom)
;;;          do (format t "   coords: ~a -> ~a~%" from-coord to-coord)
;;;          do (format t "    arrow: ~a~%" arrow)
             collect arrow
          )))

(defun cartoon-atom-map (widget atom-map from-matter to-matter)
  (let ((shape (cartoon-atom-map-arrows atom-map from-matter to-matter)))
    (safe-add-shape widget (coerce shape 'vector) :name "arrows")
    nil))

;;; Set the stereoisomer using a list of (atom-name config) pairs
;;; Example:  (set-stereoisomer-mapping *agg* '((:C1 :R) (:C2 :S))
(defun set-stereoisomer-mapping (matter atom-name-to-config)
  (loop for (name config) in atom-name-to-config          
     do (let ((atom (chem:first-atom-with-name matter name)))
	  (format t "Atom name: ~a  atom: ~a config: ~a~%" name atom config)
	  (chem:set-configuration atom config))))

;;; Set a list of stereocenters using an integer
;;; A 0 bit is :S and 1 bit is :R
;;; The value 13 is #b1101  and it sets the configuration of the atoms
;;;    to (:R :S :R :R ).
;;;  The least significant bit is the first stereocenter and the most significant bit is the last.
(defun set-stereoisomer-using-number (list-of-centers num)
  (loop for atom across list-of-centers
     for tnum = num then (ash tnum -1)
     for config = (if (= (logand 1 tnum) 0) :s :r)
     do (format t "~a -> ~a   num: ~a~%" atom config tnum)))

;;; Return a vector of stereocenters sorted by name
(defun stereocenters-sorted-by-name (matter)
  (sort (cando:gather-stereocenters matter) #'string< :key #'chem:get-name))

;;; Set all of the stereocenters to config (:S or :R)
(defun set-all-stereocenters-to (list-of-centers config &key show)
  (cando:set-stereoisomer-func list-of-centers (constantly config) :show show)
  (format t "~a stereocenters set~%" (length list-of-centers)))

(defun calculate-all-stereochemistry (vector-of-centers)
  (dotimes (i (length vector-of-centers))
    (format t "Center: ~a  config: ~a~%" (elt vector-of-centers i) (chem:calculate-stereochemical-configuration (elt vector-of-centers i)))))

;;; Return the number of stereoisomers 
(defun number-of-stereoisomers (vector-of-centers)
  (expt 2 (length vector-of-centers)))

#++(defun carboxylic-acid-atoms (matter unique-name)
  (let ((c (chem:first-atom-with-name matter unique-name))
	(carb (core:make-cxx-object 'chem:chem-info)))
    (chem:compile-smarts carb "C1(~O2)~O3")
    (or (chem:matches carb c)
	(error "The atom ~a is not a carboxylic acid carbon" c))
    (let* ((m (chem:get-match carb))
	   (o2 (chem:get-atom-with-tag m :2))
	   (o3 (chem:get-atom-with-tag m :3)))
      (list c o2 o3))))


(defun center-on (matter &optional (dest (geom:vec 0.0 0.0 0.0)))
  (let ((dest (cond
                ((typep dest 'geom:v3) dest)
                ((consp dest) (geom:vec (first dest) (second dest) (third dest)))
                (t (error "Convert ~a to vec" dest)))))
    (let ((transform (geom:make-m4-translate 
                      (geom:v- dest (chem:geometric-center matter) ))))
      (chem:apply-transform-to-atoms matter transform))))

(defun rotate-x (matter angle-degrees)
  (let ((transform (geom:make-m4-rotate-x (* 0.0174533 angle-degrees))))
    (chem:apply-transform-to-atoms matter transform)))

(defun rotate-y (matter angle-degrees)
  (let ((transform (geom:make-m4-rotate-y (* 0.0174533 angle-degrees))))
    (chem:apply-transform-to-atoms matter transform)))

(defun rotate-z (matter angle-degrees)
  (let ((transform (geom:make-m4-rotate-z (* 0.0174533 angle-degrees))))
    (chem:apply-transform-to-atoms matter transform)))

(defmacro := (a b)
  `(defparameter ,a ,b))
